// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "ArgusObjectPool.h"
#include "CoreMinimal.h"
#include <vector>

template <typename ValueComparisonType>
class IArgusKDTreeNode : public IObjectPoolable
{
	virtual FVector GetLocation() const = 0;
	virtual void Populate(const FVector& worldSpaceLocation) = 0;
	virtual bool ShouldSkipNode() const = 0;
	virtual bool ShouldSkipNode(TFunction<bool(ValueComparisonType)> queryFilter) const = 0;
	virtual bool PassesRangeCheck(const FVector& targetLocation, float rangeSquared) const = 0;
	virtual float GetValueForDimension(uint16 dimension) const = 0;
};

// Type IArgusKDTreeNode is an implicit constraint for using the ArgusKDTree. Types you want to pool should inherit from IArgusKDTreeNode 
template <class NodeType, typename ValueComparisonType>
class ArgusKDTree
{
public:
	~ArgusKDTree();

	FVector FlushAllNodes();
	void ResetKDTreeWithAverageLocation();

protected:
	void ClearNodeRecursive(NodeType* node, FVector& currentAverageLocation, uint16& priorNodeCount);
	void InsertNodeIntoKDTreeRecursive(NodeType* iterationNode, NodeType* nodeToInsert, uint16 depth);

	const NodeType* FindNodeClosestToLocationRecursive(const NodeType* iterationNode, const FVector& targetLocation, TFunction<bool(ValueComparisonType)> queryFilter, uint16 depth) const;
	const NodeType* ChooseNodeCloserToTarget(const NodeType* node0, const NodeType* node1, const FVector& targetLocation, TFunction<bool(ValueComparisonType)> queryFilter) const;

	void FindNodesWithinRangeOfLocationRecursive(TArray<const NodeType*>& outNearbyNodes, const NodeType* iterationNode, const FVector& targetLocation, const float rangeSquared, TFunction<bool(ValueComparisonType)> queryFilter, uint16 depth) const;
	void FindNodesWithinConvexPolyRecursive(TArray<const NodeType*>& outOverlappingNodes, const NodeType* iterationNode, const TArray<FVector>& convexPolygonPoints, TFunction<bool(ValueComparisonType)> queryFilter, uint16 depth) const;

	NodeType* m_rootNode = nullptr;
	ArgusObjectPool<NodeType> m_nodePool;
};

template <class NodeType, typename ValueComparisonType>
ArgusKDTree<NodeType, ValueComparisonType>::~ArgusKDTree()
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	FlushAllNodes();
	m_nodePool.ClearPool();
}

template <class NodeType, typename ValueComparisonType>
FVector ArgusKDTree<NodeType, ValueComparisonType>::FlushAllNodes()
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	ARGUS_TRACE(ArgusKDTree::FlushAllNodes);

	FVector sumLocation = FVector::ZeroVector;
	uint16 numNodes = 0u;

	if (m_rootNode)
	{
		ClearNodeRecursive(m_rootNode, sumLocation, numNodes);
		m_nodePool.Release(m_rootNode);
	}

	if (numNodes > 0u)
	{
		return  (sumLocation / static_cast<float>(numNodes));
	}
	
	return sumLocation;
}

template <class NodeType, typename ValueComparisonType>
void ArgusKDTree<NodeType, ValueComparisonType>::ResetKDTreeWithAverageLocation()
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	ARGUS_TRACE(ArgusKDTree::ResetKDTreeWithAverageLocation);

	const FVector averageLocation = FlushAllNodes();

	if (m_rootNode)
	{
		m_nodePool.Release(m_rootNode);
	}
	m_rootNode = m_nodePool.Take();
	m_rootNode->Populate(averageLocation);
}

template <class NodeType, typename ValueComparisonType>
void ArgusKDTree<NodeType, ValueComparisonType>::ClearNodeRecursive(NodeType* node, FVector& currentAverageLocation, uint16& priorNodeCount)
{
	if (!node)
	{
		return;
	}

	if (!node->ShouldSkipNode())
	{
		priorNodeCount++;
		currentAverageLocation += node->GetLocation();
	}

	if (node->m_leftChild)
	{
		ClearNodeRecursive(node->m_leftChild, currentAverageLocation, priorNodeCount);
		m_nodePool.Release(node->m_leftChild);
	}

	if (node->m_rightChild)
	{
		ClearNodeRecursive(node->m_rightChild, currentAverageLocation, priorNodeCount);
		m_nodePool.Release(node->m_rightChild);
	}
}

template <class NodeType, typename ValueComparisonType>
void ArgusKDTree<NodeType, ValueComparisonType>::InsertNodeIntoKDTreeRecursive(NodeType* iterationNode, NodeType* nodeToInsert, uint16 depth)
{
	if (!iterationNode || !nodeToInsert)
	{
		return;
	}

	uint16 dimension = depth % 3u;

	float iterationNodeValue = 0.0f;
	float noteToInsertValue = 0.0f;
	switch (dimension)
	{
		case 0:
			iterationNodeValue = iterationNode->GetLocation().X;
			noteToInsertValue = nodeToInsert->GetLocation().X;
			break;
		case 1:
			iterationNodeValue = iterationNode->GetLocation().Y;
			noteToInsertValue = nodeToInsert->GetLocation().Y;
			break;
		case 2:
			iterationNodeValue = iterationNode->GetLocation().Z;
			noteToInsertValue = nodeToInsert->GetLocation().Z;
			break;
	}

	if (noteToInsertValue < iterationNodeValue)
	{
		if (!iterationNode->m_leftChild)
		{
			iterationNode->m_leftChild = nodeToInsert;
			return;
		}

		InsertNodeIntoKDTreeRecursive(iterationNode->m_leftChild, nodeToInsert, depth + 1u);
	}
	else
	{
		if (!iterationNode->m_rightChild)
		{
			iterationNode->m_rightChild = nodeToInsert;
			return;
		}

		InsertNodeIntoKDTreeRecursive(iterationNode->m_rightChild, nodeToInsert, depth + 1u);
	}
}

template <class NodeType, typename ValueComparisonType>
const NodeType* ArgusKDTree<NodeType, ValueComparisonType>::FindNodeClosestToLocationRecursive(const NodeType* iterationNode, const FVector& targetLocation, TFunction<bool(ValueComparisonType)> queryFilter, uint16 depth) const
{
	if (!iterationNode)
	{
		return nullptr;
	}

	uint16 dimension = depth % 3u;
	float iterationNodeValue = 0.0f;
	float targetLocationValue = 0.0f;
	switch (dimension)
	{
		case 0:
			iterationNodeValue = iterationNode->GetLocation().X;
			targetLocationValue = targetLocation.X;
			break;
		case 1:
			iterationNodeValue = iterationNode->GetLocation().Y;
			targetLocationValue = targetLocation.Y;
			break;
		case 2:
			iterationNodeValue = iterationNode->GetLocation().Z;
			targetLocationValue = targetLocation.Z;
			break;
	}

	NodeType* firstBranch = nullptr;
	NodeType* secondBranch = nullptr;
	if (targetLocationValue < iterationNodeValue)
	{
		firstBranch = iterationNode->m_leftChild;
		secondBranch = iterationNode->m_rightChild;
	}
	else
	{
		firstBranch = iterationNode->m_rightChild;
		secondBranch = iterationNode->m_leftChild;
	}

	const NodeType* potentialNearestNeighbor = FindNodeClosestToLocationRecursive(firstBranch, targetLocation, queryFilter, depth + 1);
	potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, queryFilter);

	if (potentialNearestNeighbor)
	{
		const float potentialDistanceSquared = FVector::DistSquared(potentialNearestNeighbor->GetLocation(), targetLocation);
		const float distanceAlongDimensionSquared = FMath::Square(iterationNodeValue - targetLocationValue);

		if (potentialDistanceSquared > distanceAlongDimensionSquared)
		{
			const NodeType* cachedPotentialNearestNeighbor = potentialNearestNeighbor;
			potentialNearestNeighbor = FindNodeClosestToLocationRecursive(secondBranch, targetLocation, queryFilter, depth + 1);
			potentialNearestNeighbor = ChooseNodeCloserToTarget(ChooseNodeCloserToTarget(cachedPotentialNearestNeighbor, potentialNearestNeighbor, targetLocation, queryFilter), potentialNearestNeighbor, targetLocation, queryFilter);
		}
	}
	else
	{
		potentialNearestNeighbor = FindNodeClosestToLocationRecursive(secondBranch, targetLocation, queryFilter, depth + 1);
		potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, queryFilter);
	}

	return potentialNearestNeighbor;
}

template <class NodeType, typename ValueComparisonType>
const NodeType* ArgusKDTree<NodeType, ValueComparisonType>::ChooseNodeCloserToTarget(const NodeType* node0, const NodeType* node1, const FVector& targetLocation, TFunction<bool(ValueComparisonType)> queryFilter) const
{
	if (!node0 || node0->ShouldSkipNode(queryFilter))
	{
		return node1;
	}
	if (!node1 || node1->ShouldSkipNode(queryFilter))
	{
		return node0;
	}

	if (FVector::DistSquared(node0->GetLocation(), targetLocation) > FVector::DistSquared(node1->GetLocation(), targetLocation))
	{
		return node1;
	}
	else
	{
		return node0;
	}
}

template <class NodeType, typename ValueComparisonType>
void ArgusKDTree<NodeType, ValueComparisonType>::FindNodesWithinRangeOfLocationRecursive(TArray<const NodeType*>& outNearbyNodes, const NodeType* iterationNode, const FVector& targetLocation, const float rangeSquared, TFunction<bool(ValueComparisonType)> queryFilter, uint16 depth) const
{
	if (!iterationNode)
	{
		return;
	}

	if (iterationNode->PassesRangeCheck(targetLocation, rangeSquared) && !iterationNode->ShouldSkipNode(queryFilter))
	{
		outNearbyNodes.Add(iterationNode);
	}

	if (iterationNode->forceFullSearch)
	{
		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_leftChild, targetLocation, rangeSquared, queryFilter, depth + 1);
		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_rightChild, targetLocation, rangeSquared, queryFilter, depth + 1);
		return;
	}

	uint16 dimension = (depth) % 3u;
	float targetLocationValue = 0.0f;
	switch (dimension)
	{
		case 0:
			targetLocationValue = targetLocation.X;
			break;
		case 1:
			targetLocationValue = targetLocation.Y;
			break;
		case 2:
			targetLocationValue = targetLocation.Z;
			break;
	}

	const float differenceInDimension = targetLocationValue - iterationNode->GetValueForDimension(dimension);
	if (FMath::Square(differenceInDimension) < rangeSquared)
	{
		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_leftChild, targetLocation, rangeSquared, queryFilter, depth + 1);
		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_rightChild, targetLocation, rangeSquared, queryFilter, depth + 1);
	}
	else
	{
		if (differenceInDimension < 0.0f)
		{
			FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_leftChild, targetLocation, rangeSquared, queryFilter, depth + 1);
		}
		else
		{
			FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_rightChild, targetLocation, rangeSquared, queryFilter, depth + 1);
		}
	}
}

template <class NodeType, typename ValueComparisonType>
void ArgusKDTree<NodeType, ValueComparisonType>::FindNodesWithinConvexPolyRecursive(TArray<const NodeType*>& outNearbyNodes, const NodeType* iterationNode, const TArray<FVector>& convexPolygonPoints, TFunction<bool(ValueComparisonType)> queryFilter, uint16 depth) const
{
	if (!iterationNode)
	{
		return;
	}

	uint16 dimension = (depth) % 3u;
	bool allDifferencesSameSign = false;
	bool isInside = true;
	float minDifferenceInDimension = FLT_MAX;
	float currentSign = 0.0f;
	for (int32 i = 0; i < convexPolygonPoints.Num(); ++i)
	{
		if (isInside && ArgusMath::IsLeftOfUnreal(FVector2D(convexPolygonPoints[i]), FVector2D(convexPolygonPoints[(i + 1) % convexPolygonPoints.Num()]), FVector2D(iterationNode->GetLocation())))
		{
			isInside = false;
			break;
		}

		float pointValueInDimension = 0.0f;
		switch (dimension)
		{
			case 0:
				pointValueInDimension = convexPolygonPoints[i].X;
				break;
			case 1:
				pointValueInDimension = convexPolygonPoints[i].Y;
				break;
			case 2:
				continue;
		}

		const float differenceInDimension = pointValueInDimension - iterationNode->GetValueForDimension(dimension);
		if (i == 0)
		{
			currentSign = FMath::Sign(differenceInDimension);
		}
		else if (allDifferencesSameSign)
		{
			if (FMath::Sign(differenceInDimension) != currentSign)
			{
				allDifferencesSameSign = false;
			}
			else if (FMath::Abs(differenceInDimension) < FMath::Abs(minDifferenceInDimension))
			{
				minDifferenceInDimension = differenceInDimension;
			}
		}
	}

	if (isInside && !iterationNode->ShouldSkipNode(queryFilter))
	{
		outNearbyNodes.Add(iterationNode);
	}

	if (iterationNode->forceFullSearch || dimension == 2 || !allDifferencesSameSign)
	{
		FindNodesWithinConvexPolyRecursive(outNearbyNodes, iterationNode->m_leftChild, convexPolygonPoints, queryFilter, depth + 1);
		FindNodesWithinConvexPolyRecursive(outNearbyNodes, iterationNode->m_rightChild, convexPolygonPoints, queryFilter, depth + 1);
		return;
	}

	if (minDifferenceInDimension < 0.0f)
	{
		FindNodesWithinConvexPolyRecursive(outNearbyNodes, iterationNode->m_leftChild, convexPolygonPoints, queryFilter, depth + 1);
	}
	else
	{
		FindNodesWithinConvexPolyRecursive(outNearbyNodes, iterationNode->m_rightChild, convexPolygonPoints, queryFilter, depth + 1);
	}
}
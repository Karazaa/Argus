// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusObjectPool.h"
#include "CoreMinimal.h"
#include <vector>

class ArgusEntity;

class IArgusKDTreeNode : public IObjectPoolable
{
};

// Type IArgusKDTreeNode is an implicit constraint for using the ArgusKDTree. Types you want to pool should inherit from IArgusKDTreeNode 
template <class NodeType>
class ArgusKDTree
{
public:
	~ArgusKDTree();

	FVector FlushAllNodes();
	void ResetKDTreeWithAverageLocation();

protected:
	void ClearNodeRecursive(NodeType* node, FVector& currentAverageLocation, uint16& priorNodeCount);
	void InsertNodeIntoKDTreeRecursive(NodeType* iterationNode, NodeType* nodeToInsert, uint16 depth);

	const NodeType* FindNodeClosestToLocationRecursive(const NodeType* iterationNode, const FVector& targetLocation, uint16 entityIdToIgnore, uint16 depth) const;
	const NodeType* ChooseNodeCloserToTarget(const NodeType* node0, const NodeType* node1, const FVector& targetLocation, uint16 entityIdToIgnore) const;

	void FindNodesWithinRangeOfLocationRecursive(std::vector<const NodeType*>& outNearbyNodes, const NodeType* iterationNode, const FVector& targetLocation, const float rangeSquared, uint16 entityIdToIgnore, uint16 depth) const;
	
	NodeType* m_rootNode = nullptr;
	ArgusObjectPool<NodeType> m_nodePool;
};

template <class NodeType>
ArgusKDTree<NodeType>::~ArgusKDTree()
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	FlushAllNodes();
	m_nodePool.ClearPool();
}

template <class NodeType>
FVector ArgusKDTree<NodeType>::FlushAllNodes()
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

	return  (sumLocation / static_cast<float>(numNodes));
}

template <class NodeType>
void ArgusKDTree<NodeType>::ResetKDTreeWithAverageLocation()
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

template <class NodeType>
void ArgusKDTree<NodeType>::ClearNodeRecursive(NodeType* node, FVector& currentAverageLocation, uint16& priorNodeCount)
{
	if (!node)
	{
		return;
	}

	if (node->m_entityId != ArgusECSConstants::k_maxEntities)
	{
		priorNodeCount++;
		currentAverageLocation += node->m_worldSpaceLocation;
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

template <class NodeType>
void ArgusKDTree<NodeType>::InsertNodeIntoKDTreeRecursive(NodeType* iterationNode, NodeType* nodeToInsert, uint16 depth)
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
		iterationNodeValue = iterationNode->m_worldSpaceLocation.X;
		noteToInsertValue = nodeToInsert->m_worldSpaceLocation.X;
		break;
	case 1:
		iterationNodeValue = iterationNode->m_worldSpaceLocation.Y;
		noteToInsertValue = nodeToInsert->m_worldSpaceLocation.Y;
		break;
	case 2:
		iterationNodeValue = iterationNode->m_worldSpaceLocation.Z;
		noteToInsertValue = nodeToInsert->m_worldSpaceLocation.Z;
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

template <class NodeType>
const NodeType* ArgusKDTree<NodeType>::FindNodeClosestToLocationRecursive(const NodeType* iterationNode, const FVector& targetLocation, uint16 entityIdToIgnore, uint16 depth) const
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
		iterationNodeValue = iterationNode->m_worldSpaceLocation.X;
		targetLocationValue = targetLocation.X;
		break;
	case 1:
		iterationNodeValue = iterationNode->m_worldSpaceLocation.Y;
		targetLocationValue = targetLocation.Y;
		break;
	case 2:
		iterationNodeValue = iterationNode->m_worldSpaceLocation.Z;
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

	const NodeType* potentialNearestNeighbor = FindNodeClosestToLocationRecursive(firstBranch, targetLocation, entityIdToIgnore, depth + 1);
	potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, entityIdToIgnore);

	if (potentialNearestNeighbor)
	{
		const float potentialDistanceSquared = FVector::DistSquared(potentialNearestNeighbor->m_worldSpaceLocation, targetLocation);
		const float distanceAlongDimensionSquared = FMath::Square(iterationNodeValue - targetLocationValue);

		if (potentialDistanceSquared > distanceAlongDimensionSquared)
		{
			const NodeType* cachedPotentialNearestNeighbor = potentialNearestNeighbor;
			potentialNearestNeighbor = FindNodeClosestToLocationRecursive(secondBranch, targetLocation, entityIdToIgnore, depth + 1);
			potentialNearestNeighbor = ChooseNodeCloserToTarget(ChooseNodeCloserToTarget(cachedPotentialNearestNeighbor, potentialNearestNeighbor, targetLocation, entityIdToIgnore), potentialNearestNeighbor, targetLocation, entityIdToIgnore);
		}
	}
	else
	{
		potentialNearestNeighbor = FindNodeClosestToLocationRecursive(secondBranch, targetLocation, entityIdToIgnore, depth + 1);
		potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, entityIdToIgnore);
	}

	return potentialNearestNeighbor;
}

template <class NodeType>
const NodeType* ArgusKDTree<NodeType>::ChooseNodeCloserToTarget(const NodeType* node0, const NodeType* node1, const FVector& targetLocation, uint16 entityIdToIgnore) const
{
	if (!node0 || node0->m_entityId == entityIdToIgnore || node0->m_entityId == ArgusECSConstants::k_maxEntities)
	{
		return node1;
	}
	if (!node1 || node1->m_entityId == entityIdToIgnore || node1->m_entityId == ArgusECSConstants::k_maxEntities)
	{
		return node0;
	}

	if (FVector::DistSquared(node0->m_worldSpaceLocation, targetLocation) > FVector::DistSquared(node1->m_worldSpaceLocation, targetLocation))
	{
		return node1;
	}
	else
	{
		return node0;
	}
}

template <class NodeType>
void ArgusKDTree<NodeType>::FindNodesWithinRangeOfLocationRecursive(std::vector<const NodeType*>& outNearbyNodes, const NodeType* iterationNode, const FVector& targetLocation, const float rangeSquared, uint16 entityIdToIgnore, uint16 depth) const
{
	if (!iterationNode)
	{
		return;
	}

	if (FVector::DistSquared(iterationNode->m_worldSpaceLocation, targetLocation) < rangeSquared)
	{
		if (iterationNode->m_entityId != entityIdToIgnore && iterationNode->m_entityId != ArgusECSConstants::k_maxEntities)
		{
			outNearbyNodes.push_back(iterationNode);
		}

		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_rightChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_leftChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		return;
	}

	uint16 dimension = depth % 3u;
	float iterationNodeValue = 0.0f;
	float targetLocationValue = 0.0f;
	switch (dimension)
	{
	case 0:
		iterationNodeValue = iterationNode->m_worldSpaceLocation.X;
		targetLocationValue = targetLocation.X;
		break;
	case 1:
		iterationNodeValue = iterationNode->m_worldSpaceLocation.Y;
		targetLocationValue = targetLocation.Y;
		break;
	case 2:
		iterationNodeValue = iterationNode->m_worldSpaceLocation.Z;
		targetLocationValue = targetLocation.Z;
		break;
	}

	const float differenceAlongDimension = targetLocationValue - iterationNodeValue;
	if (FMath::Square(differenceAlongDimension) > rangeSquared)
	{
		if (differenceAlongDimension > 0.0f)
		{
			FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_rightChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		}
		else
		{
			FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_leftChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		}
	}
	else
	{
		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_rightChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		FindNodesWithinRangeOfLocationRecursive(outNearbyNodes, iterationNode->m_leftChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
	}
}
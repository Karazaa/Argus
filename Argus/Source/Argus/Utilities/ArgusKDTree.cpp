// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusKDTree.h"
#include "ArgusEntity.h"
#include "ArgusMacros.h"
#include "ComponentDefinitions/TransformComponent.h"

void ArgusKDTree::ArgusKDTreeNode::Populate(const FVector& worldSpaceLocation)
{
	m_worldSpaceLocation = worldSpaceLocation;
}

void ArgusKDTree::ArgusKDTreeNode::Populate(const ArgusEntity& entityToRepresent)
{
	if (!entityToRepresent)
	{
		return;
	}

	const TransformComponent* transformComponent = entityToRepresent.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	m_worldSpaceLocation = transformComponent->m_transform.GetLocation();
	m_entityId = entityToRepresent.GetId();
}

void ArgusKDTree::ClearNodeRecursive(ArgusKDTreeNode* node, FVector& currentAverageLocation, uint16& priorNodeCount)
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
		node->m_leftChild = nullptr;
	}

	if (node->m_rightChild)
	{
		ClearNodeRecursive(node->m_rightChild, currentAverageLocation, priorNodeCount);
		m_nodePool.Release(node->m_rightChild);
		node->m_rightChild = nullptr;
	}
}

void ArgusKDTree::InsertNodeIntoKDTreeRecursive(ArgusKDTreeNode* iterationNode, ArgusKDTreeNode* nodeToInsert, uint16 depth)
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

bool ArgusKDTree::SearchForEntityIdRecursive(const ArgusKDTreeNode* node, uint16 entityId) const
{
	if (!node)
	{
		return false;
	}

	if (node->m_entityId == entityId)
	{
		return true;
	}

	if (node->m_leftChild && SearchForEntityIdRecursive(node->m_leftChild, entityId))
	{
		return true;
	}

	if (node->m_rightChild && SearchForEntityIdRecursive(node->m_rightChild, entityId))
	{
		return true;
	}

	return false;
}

const ArgusKDTree::ArgusKDTreeNode* ArgusKDTree::FindArgusEntityIdClosestToLocationRecursive(const ArgusKDTreeNode* iterationNode, const FVector& targetLocation, uint16 entityIdToIgnore, uint16 depth) const
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

	ArgusKDTreeNode* firstBranch = nullptr;
	ArgusKDTreeNode* secondBranch = nullptr;
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

	const ArgusKDTreeNode* potentialNearestNeighbor = FindArgusEntityIdClosestToLocationRecursive(firstBranch, targetLocation, entityIdToIgnore, depth + 1);
	potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, entityIdToIgnore);

	if (potentialNearestNeighbor)
	{
		const float potentialDistanceSquared = FVector::DistSquared(potentialNearestNeighbor->m_worldSpaceLocation, targetLocation);
		const float distanceAlongDimensionSquared = FMath::Square(iterationNodeValue - targetLocationValue);

		if (potentialDistanceSquared > distanceAlongDimensionSquared)
		{
			const ArgusKDTreeNode* cachedPotentialNearestNeighbor = potentialNearestNeighbor;
			potentialNearestNeighbor = FindArgusEntityIdClosestToLocationRecursive(secondBranch, targetLocation, entityIdToIgnore, depth + 1);
			potentialNearestNeighbor = ChooseNodeCloserToTarget(ChooseNodeCloserToTarget(cachedPotentialNearestNeighbor, potentialNearestNeighbor, targetLocation, entityIdToIgnore), potentialNearestNeighbor, targetLocation, entityIdToIgnore);
		}
	}
	else
	{
		potentialNearestNeighbor = FindArgusEntityIdClosestToLocationRecursive(secondBranch, targetLocation, entityIdToIgnore, depth + 1);
		potentialNearestNeighbor = ChooseNodeCloserToTarget(iterationNode, potentialNearestNeighbor, targetLocation, entityIdToIgnore);
	}

	return potentialNearestNeighbor;
}

const ArgusKDTree::ArgusKDTreeNode* ArgusKDTree::ChooseNodeCloserToTarget(const ArgusKDTreeNode* node0, const ArgusKDTreeNode* node1, const FVector& targetLocation, uint16 entityIdToIgnore) const
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

void ArgusKDTree::FindArgusEntityIdsWithinRangeOfLocationRecursive(std::vector<uint16>& outNearbyArgusEntityIds, const ArgusKDTreeNode* iterationNode, const FVector& targetLocation, const float rangeSquared, uint16 entityIdToIgnore, uint16 depth) const
{
	if (!iterationNode)
	{
		return;
	}

	if (FVector::DistSquared(iterationNode->m_worldSpaceLocation, targetLocation) < rangeSquared)
	{
		if (iterationNode->m_entityId != entityIdToIgnore && iterationNode->m_entityId != ArgusECSConstants::k_maxEntities)
		{
			outNearbyArgusEntityIds.push_back(iterationNode->m_entityId);
		}
		
		FindArgusEntityIdsWithinRangeOfLocationRecursive(outNearbyArgusEntityIds, iterationNode->m_rightChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		FindArgusEntityIdsWithinRangeOfLocationRecursive(outNearbyArgusEntityIds, iterationNode->m_leftChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
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
			FindArgusEntityIdsWithinRangeOfLocationRecursive(outNearbyArgusEntityIds, iterationNode->m_rightChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		}
		else
		{
			FindArgusEntityIdsWithinRangeOfLocationRecursive(outNearbyArgusEntityIds, iterationNode->m_leftChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		}
	}
	else
	{
		FindArgusEntityIdsWithinRangeOfLocationRecursive(outNearbyArgusEntityIds, iterationNode->m_rightChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
		FindArgusEntityIdsWithinRangeOfLocationRecursive(outNearbyArgusEntityIds, iterationNode->m_leftChild, targetLocation, rangeSquared, entityIdToIgnore, depth + 1);
	}
}

ArgusKDTree::~ArgusKDTree()
{
	FlushAllNodes();
	m_nodePool.ClearPool();
}

FVector ArgusKDTree::FlushAllNodes()
{
	ARGUS_TRACE(ArgusKDTree::FlushAllNodes)

	FVector sumLocation = FVector::ZeroVector;
	uint16 numNodes = 0u;

	if (m_rootNode)
	{
		ClearNodeRecursive(m_rootNode, sumLocation, numNodes);
		m_nodePool.Release(m_rootNode);
		m_rootNode = nullptr;
	}

	return  (sumLocation / static_cast<float>(numNodes));
}

void ArgusKDTree::InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent)
{
	ARGUS_TRACE(ArgusKDTree::InsertArgusEntityIntoKDTree)

	if (!entityToRepresent)
	{
		return;
	}

	const TransformComponent* transformComponent = entityToRepresent.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	ArgusKDTreeNode* nodeToInsert = m_nodePool.Take();;
	nodeToInsert->Populate(entityToRepresent);
	if (!m_rootNode)
	{
		m_rootNode = nodeToInsert;
		return;
	}

	InsertNodeIntoKDTreeRecursive(m_rootNode, nodeToInsert, 0u);
}

void ArgusKDTree::RebuildKDTreeForAllArgusEntities()
{
	ARGUS_TRACE(ArgusKDTree::RebuildKDTreeForAllArgusEntities)

	const FVector averageLocation = FlushAllNodes();

	if (m_rootNode)
	{
		m_nodePool.Release(m_rootNode);
	}
	m_rootNode = m_nodePool.Take();
	m_rootNode->Populate(averageLocation);

	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		if (ArgusEntity retrievedEntity = ArgusEntity::RetrieveEntity(i))
		{
			InsertArgusEntityIntoKDTree(retrievedEntity);
		}
	}
}

bool ArgusKDTree::DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const
{
	if (!entityToRepresent || !m_rootNode)
	{
		return false;
	}

	return SearchForEntityIdRecursive(m_rootNode, entityToRepresent.GetId());
}

uint16 ArgusKDTree::FindArgusEntityIdClosestToLocation(const FVector& location) const
{
	return FindArgusEntityIdClosestToLocation(location, ArgusEntity::s_emptyEntity);
}

uint16 ArgusKDTree::FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const
{
	if (!m_rootNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	const ArgusKDTreeNode* foundNode = FindArgusEntityIdClosestToLocationRecursive(m_rootNode, location, entityToIgnore.GetId(), 0u);

	if (!foundNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	return foundNode->m_entityId;
}

uint16 ArgusKDTree::FindOtherArgusEntityIdClosestArgusEntity(const ArgusEntity& entityToSearchAround) const
{
	const TransformComponent* transformComponent = entityToSearchAround.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	return FindArgusEntityIdClosestToLocation(transformComponent->m_transform.GetLocation(), entityToSearchAround);
}

bool ArgusKDTree::FindArgusEntityIdsWithinRangeOfLocation(std::vector<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range) const
{
	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, location, range, ArgusEntity::s_emptyEntity);
}

bool ArgusKDTree::FindArgusEntityIdsWithinRangeOfLocation(std::vector<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const
{
	if (!m_rootNode || range <= 0.0f)
	{
		return false;
	}

	FindArgusEntityIdsWithinRangeOfLocationRecursive(outNearbyArgusEntityIds, m_rootNode, location, FMath::Square(range), entityToIgnore.GetId(), 0u);

	return outNearbyArgusEntityIds.size() > 0u;
}

bool ArgusKDTree::FindOtherArgusEntityIdsWithinRangeOfArgusEntity(std::vector<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range) const
{
	const TransformComponent* transformComponent = entityToSearchAround.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return false;
	}

	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, transformComponent->m_transform.GetLocation(), range, entityToSearchAround);
}

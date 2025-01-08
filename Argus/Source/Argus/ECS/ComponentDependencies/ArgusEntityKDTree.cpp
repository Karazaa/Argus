// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityKDTree.h"
#include "ArgusEntity.h"
#include "ComponentDefinitions/TransformComponent.h"

void ArgusEntityKDTreeNode::Populate(const FVector& worldSpaceLocation)
{
	m_worldSpaceLocation = worldSpaceLocation;
}

void ArgusEntityKDTreeNode::Populate(const ArgusEntity& entityToRepresent)
{
	if (!entityToRepresent)
	{
		ArgusEntityKDTree::ErrorOnInvalidArgusEntity(ARGUS_FUNCNAME);
		return;
	}

	const TransformComponent* transformComponent = entityToRepresent.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		ArgusEntityKDTree::ErrorOnInvalidTransformComponent(ARGUS_FUNCNAME);
		return;
	}

	m_worldSpaceLocation = transformComponent->m_transform.GetLocation();
	m_entityId = entityToRepresent.GetId();
}

void ArgusEntityKDTreeNode::Reset()
{
	m_worldSpaceLocation = FVector::ZeroVector;
	m_entityId = ArgusECSConstants::k_maxEntities;
	m_leftChild = nullptr;
	m_rightChild = nullptr;
}

bool ArgusEntityKDTreeNode::ShouldSkipNode(uint16 valueToSkip) const
{
	return m_entityId == valueToSkip || m_entityId == ArgusECSConstants::k_maxEntities;
}

void ArgusEntityKDTree::ErrorOnInvalidArgusEntity(const WIDECHAR* functionName)
{
	ARGUS_LOG
	(
		ArgusUtilitiesLog,
		Error,
		TEXT("[%s] Passed in %s is invalid. It cannot be added to or retrieved from %s."),
		functionName,
		ARGUS_NAMEOF(ArgusEntity),
		ARGUS_NAMEOF(ArgusEntityKDTree)
	);
}

void ArgusEntityKDTree::ErrorOnInvalidTransformComponent(const WIDECHAR* functionName)
{
	ARGUS_LOG
	(
		ArgusUtilitiesLog,
		Error,
		TEXT("[%s] Retrieved %s is invalid. Its owning %s cannot be added to or retrieved from %s."),
		functionName,
		ARGUS_NAMEOF(TransformComponent),
		ARGUS_NAMEOF(ArgusEntity),
		ARGUS_NAMEOF(ArgusEntityKDTree)
	);
}

void ArgusEntityKDTree::RebuildKDTreeForAllArgusEntities()
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	ARGUS_TRACE(ArgusKDTree::RebuildKDTreeForAllArgusEntities);

	ResetKDTreeWithAverageLocation();

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity retrievedEntity = ArgusEntity::RetrieveEntity(i);
		if (!retrievedEntity)
		{
			continue;
		}

		const TransformComponent* transformComponent = retrievedEntity.GetComponent<TransformComponent>();
		if (!transformComponent)
		{
			continue;
		}

		InsertArgusEntityIntoKDTree(retrievedEntity);
	}
}

void ArgusEntityKDTree::InsertArgusEntityIntoKDTree(const ArgusEntity& entityToRepresent)
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	ARGUS_TRACE(ArgusKDTree::InsertArgusEntityIntoKDTree);

	if (!entityToRepresent)
	{
		ErrorOnInvalidArgusEntity(ARGUS_FUNCNAME);
		return;
	}

	const TransformComponent* transformComponent = entityToRepresent.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		ErrorOnInvalidTransformComponent(ARGUS_FUNCNAME);
		return;
	}

	ArgusEntityKDTreeNode* nodeToInsert = m_nodePool.Take();
	nodeToInsert->Populate(entityToRepresent);
	if (!m_rootNode)
	{
		m_rootNode = nodeToInsert;
		return;
	}

	InsertNodeIntoKDTreeRecursive(m_rootNode, nodeToInsert, 0u);
}

uint16 ArgusEntityKDTree::FindArgusEntityIdClosestToLocation(const FVector& location) const
{
	return FindArgusEntityIdClosestToLocation(location, ArgusEntity::k_emptyEntity);
}

uint16 ArgusEntityKDTree::FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const
{
	if (!m_rootNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	const ArgusEntityKDTreeNode* foundNode = FindNodeClosestToLocationRecursive(m_rootNode, location, entityToIgnore.GetId(), 0u);

	if (!foundNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	return foundNode->m_entityId;
}

uint16 ArgusEntityKDTree::FindOtherArgusEntityIdClosestArgusEntity(const ArgusEntity& entityToSearchAround) const
{
	if (!entityToSearchAround)
	{
		ErrorOnInvalidArgusEntity(ARGUS_FUNCNAME);
		return ArgusECSConstants::k_maxEntities;
	}

	const TransformComponent* transformComponent = entityToSearchAround.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		ErrorOnInvalidTransformComponent(ARGUS_FUNCNAME);
		return ArgusECSConstants::k_maxEntities;
	}

	return FindArgusEntityIdClosestToLocation(transformComponent->m_transform.GetLocation(), entityToSearchAround);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(std::vector<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range) const
{
	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, location, range, ArgusEntity::k_emptyEntity);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(std::vector<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const
{
	if (!m_rootNode)
	{
		return false;
	}

	if (range <= 0.0f)
	{
		ARGUS_LOG(ArgusUtilitiesLog, Error, TEXT("[%s] Searching range is less than or equal to 0."), ARGUS_FUNCNAME);
		return false;
	}

	std::vector<const ArgusEntityKDTreeNode*> foundNodes;
	FindNodesWithinRangeOfLocationRecursive(foundNodes, m_rootNode, location, FMath::Square(range), entityToIgnore.GetId(), 0u);
	outNearbyArgusEntityIds.reserve(foundNodes.size());
	for (int i = 0; i < foundNodes.size(); ++i)
	{
		if (!foundNodes[i])
		{
			continue;
		}

		outNearbyArgusEntityIds.push_back(foundNodes[i]->m_entityId);
	}

	return outNearbyArgusEntityIds.size() > 0u;
}

bool ArgusEntityKDTree::FindOtherArgusEntityIdsWithinRangeOfArgusEntity(std::vector<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range) const
{
	if (!entityToSearchAround)
	{
		ErrorOnInvalidArgusEntity(ARGUS_FUNCNAME);
		return false;
	}

	const TransformComponent* transformComponent = entityToSearchAround.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		ErrorOnInvalidTransformComponent(ARGUS_FUNCNAME);
		return false;
	}

	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, transformComponent->m_transform.GetLocation(), range, entityToSearchAround);
}

bool ArgusEntityKDTree::DoesArgusEntityExistInKDTree(const ArgusEntity& entityToRepresent) const
{
	if (!entityToRepresent)
	{
		ErrorOnInvalidArgusEntity(ARGUS_FUNCNAME);
		return false;
	}

	if (!m_rootNode)
	{
		return false;
	}

	return SearchForEntityIdRecursive(m_rootNode, entityToRepresent.GetId());
}

bool ArgusEntityKDTree::SearchForEntityIdRecursive(const ArgusEntityKDTreeNode* node, uint16 entityId) const
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
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

	m_worldSpaceLocation = transformComponent->m_location;
	m_entityId = entityToRepresent.GetId();
}

void ArgusEntityKDTreeNode::Reset()
{
	m_worldSpaceLocation = FVector::ZeroVector;
	m_entityId = ArgusECSConstants::k_maxEntities;
	m_leftChild = nullptr;
	m_rightChild = nullptr;
}

bool ArgusEntityKDTreeNode::ShouldSkipNode() const
{
	return m_entityId == ArgusECSConstants::k_maxEntities;
}

bool ArgusEntityKDTreeNode::ShouldSkipNode(TFunction<bool(uint16)> queryFilter) const
{
	if (m_entityId == ArgusECSConstants::k_maxEntities)
	{
		return true;
	}

	if (!queryFilter)
	{
		return false;
	}

	return !queryFilter(m_entityId);
}

bool ArgusEntityKDTreeNode::PassesRangeCheck(const FVector& targetLocation, float rangeSquared, float& nodeRangeSquared) const
{
	nodeRangeSquared = FVector::DistSquared(GetLocation(), targetLocation);
	return nodeRangeSquared < rangeSquared;
}

float ArgusEntityKDTreeNode::GetValueForDimension(uint16 dimension) const
{
	switch (dimension)
	{
		case 0:
			return m_worldSpaceLocation.X;
		case 1:
			return m_worldSpaceLocation.Y;
		case 2:
			return m_worldSpaceLocation.Z;
		default: 
			return 0.0f;
	}
}

void ArgusEntityKDTreeRangeOutput::Add(const ArgusEntityKDTreeNode* nodeToAdd, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, float distFromTargetSquared)
{
	if (!nodeToAdd)
	{
		return;
	}

	const uint16 entityId = nodeToAdd->m_entityId;
	if (distFromTargetSquared < thresholds.m_avoidanceRangeThresholdSquared)
	{
		m_entityIdsWithinAvoidanceRange.Add(entityId);
	}

	if (distFromTargetSquared < thresholds.m_meleeRangeThresholdSquared)
	{
		m_entityIdsWithinMeleeRange.Add(entityId);
	}
	else if (distFromTargetSquared < thresholds.m_rangedRangeThresholdSquared)
	{
		m_entityIdsWithinRangedRange.Add(entityId);
	}
	else
	{
		m_entityIdsWithinSightRange.Add(entityId);
	}
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

void ArgusEntityKDTree::SeedTreeWithAverageEntityLocation()
{
	FlushAllNodes();

	FVector averageLocation = FVector::ZeroVector;
	float numIncludedEntities = 0.0f;
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

		averageLocation += transformComponent->m_location;
		numIncludedEntities += 1.0f;
	}

	averageLocation /= numIncludedEntities;

	if (m_rootNode)
	{
		m_nodePool.Release(m_rootNode);
	}
	m_rootNode = m_nodePool.Take();
	m_rootNode->Populate(averageLocation);
}

void ArgusEntityKDTree::InsertAllArgusEntitiesIntoKDTree()
{
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

void ArgusEntityKDTree::RebuildKDTreeForAllArgusEntities()
{
	ARGUS_MEMORY_TRACE(ArgusKDTree);
	ARGUS_TRACE(ArgusKDTree::RebuildKDTreeForAllArgusEntities);

	if (m_rootNode)
	{
		RebuildSubTreeForArgusEntitiesRecursive(m_rootNode, false);
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
	const uint16 entityIdToIgnore = entityToIgnore.GetId();
	TFunction<bool(uint16)> predicate = nullptr;
	if (entityIdToIgnore != ArgusECSConstants::k_maxEntities)
	{
		predicate = [entityIdToIgnore](uint16 valueToSkip) { return valueToSkip != entityIdToIgnore; };
	}

	return FindArgusEntityIdClosestToLocation(location, predicate);
}

uint16 ArgusEntityKDTree::FindArgusEntityIdClosestToLocation(const FVector& location, const TFunction<bool(uint16)> queryFilter) const
{
	if (!m_rootNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	const ArgusEntityKDTreeNode* foundNode = FindNodeClosestToLocationRecursive(m_rootNode, location, queryFilter, 0u);

	if (!foundNode)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	return foundNode->m_entityId;
}

uint16 ArgusEntityKDTree::FindOtherArgusEntityIdClosestToArgusEntity(const ArgusEntity& entityToSearchAround, const TFunction<bool(uint16)> queryFilterOverride) const
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

	if (queryFilterOverride)
	{
		return FindArgusEntityIdClosestToLocation(transformComponent->m_location, queryFilterOverride);
	}

	return FindArgusEntityIdClosestToLocation(transformComponent->m_location, entityToSearchAround);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range) const
{
	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, location, range, ArgusEntity::k_emptyEntity);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const
{
	if (!m_rootNode)
	{
		return false;
	}

	uint16 entityIdToIgnore = entityToIgnore.GetId();
	TFunction<bool(uint16)> predicate = nullptr;
	if (entityIdToIgnore != ArgusECSConstants::k_maxEntities)
	{
		predicate = [entityIdToIgnore](uint16 valueToSkip) { return valueToSkip != entityIdToIgnore; };
	}

	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, location, range, predicate);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const TFunction<bool(uint16)> queryFilterOverride) const
{
	if (range <= 0.0f)
	{
		ARGUS_LOG(ArgusUtilitiesLog, Error, TEXT("[%s] Searching range is less than or equal to 0."), ARGUS_FUNCNAME);
		return false;
	}

	ArgusEntityKDTreeRangeOutput foundEntities;
	FindNodesWithinRangeOfLocationRecursive(foundEntities, ArgusEntityKDTreeQueryRangeThresholds(0.0f, 0.0f, 0.0f), m_rootNode, location, FMath::Square(range), queryFilterOverride, 0u);
	outNearbyArgusEntityIds.Reserve(foundEntities.m_entityIdsWithinSightRange.Num());
	for (int32 i = 0; i < foundEntities.m_entityIdsWithinSightRange.Num(); ++i)
	{
		outNearbyArgusEntityIds.Add(foundEntities.m_entityIdsWithinSightRange[i]);
	}

	return outNearbyArgusEntityIds.Num() > 0u;
}

bool ArgusEntityKDTree::FindOtherArgusEntityIdsWithinRangeOfArgusEntity(TArray<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range, const TFunction<bool(uint16)> queryFilterOverride) const
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

	if (queryFilterOverride)
	{
		return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, transformComponent->m_location, range, queryFilterOverride);
	}

	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, transformComponent->m_location, range, entityToSearchAround);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector>& convexPolygonPoints) const
{
	if (!m_rootNode)
	{
		return false;
	}

	if (convexPolygonPoints.Num() < 3)
	{
		ARGUS_LOG(ArgusUtilitiesLog, Error, TEXT("[%s] Number of points in %s is less than three. You can't have a polygon with fewer than three points."), ARGUS_FUNCNAME, ARGUS_NAMEOF(convexPolygonPoints));
		return false;
	}

	ArgusEntityKDTreeRangeOutput foundEntities;
	FindNodesWithinConvexPolyRecursive(foundEntities, ArgusEntityKDTreeQueryRangeThresholds(0.0f, 0.0f, 0.0f), m_rootNode, convexPolygonPoints, nullptr, 0u);
	outNearbyArgusEntityIds.Reserve(foundEntities.m_entityIdsWithinSightRange.Num());
	for (int32 i = 0; i < foundEntities.m_entityIdsWithinSightRange.Num(); ++i)
	{
		outNearbyArgusEntityIds.Add(foundEntities.m_entityIdsWithinSightRange[i]);
	}

	return outNearbyArgusEntityIds.Num() > 0;
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector2D>& convexPolygonPoints) const
{
	TArray<FVector> convexPolyPoints3D;
	convexPolyPoints3D.Reserve(convexPolygonPoints.Num());
	for (int32 i = 0; i < convexPolygonPoints.Num(); ++i)
	{
		convexPolyPoints3D.Add(FVector(convexPolygonPoints[i], 0.0f));
	}
	return FindArgusEntityIdsWithinConvexPoly(outNearbyArgusEntityIds, convexPolyPoints3D);
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

void ArgusEntityKDTree::RebuildSubTreeForArgusEntitiesRecursive(ArgusEntityKDTreeNode*& node, bool forceReInsertChildren)
{
	if (!node)
	{
		return;
	}

	if (forceReInsertChildren)
	{
		ArgusEntityKDTreeNode* leftChild = node->m_leftChild;
		ArgusEntityKDTreeNode* rightChild = node->m_rightChild;
		ClearNodeWithReInsert(node);
		RebuildSubTreeForArgusEntitiesRecursive(leftChild, true);
		RebuildSubTreeForArgusEntitiesRecursive(rightChild, true);
		return;
	}

	if (node->ShouldSkipNode())
	{
		RebuildSubTreeForArgusEntitiesRecursive(node->m_leftChild, false);
		RebuildSubTreeForArgusEntitiesRecursive(node->m_rightChild, false);
		return;
	}

	const ArgusEntity entity = ArgusEntity::RetrieveEntity(node->m_entityId);
	if (!entity)
	{
		return;
	}

	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	if (transformComponent->m_location != node->m_worldSpaceLocation)
	{
		ArgusEntityKDTreeNode* leftChild = node->m_leftChild;
		ArgusEntityKDTreeNode* rightChild = node->m_rightChild;
		ClearNodeWithReInsert(node);
		RebuildSubTreeForArgusEntitiesRecursive(leftChild, true);
		RebuildSubTreeForArgusEntitiesRecursive(rightChild, true);
	}
	else
	{
		RebuildSubTreeForArgusEntitiesRecursive(node->m_leftChild, false);
		RebuildSubTreeForArgusEntitiesRecursive(node->m_rightChild, false);
	}
}

void ArgusEntityKDTree::ClearNodeWithReInsert(ArgusEntityKDTreeNode*& node)
{
	const ArgusEntity entity = ArgusEntity::RetrieveEntity(node->m_entityId);
	if (!entity)
	{
		return;
	}

	m_nodePool.Release(node);
	InsertArgusEntityIntoKDTree(entity);
}

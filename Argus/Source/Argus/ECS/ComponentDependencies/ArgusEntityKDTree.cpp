// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityKDTree.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
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
	ARGUS_RETURN_ON_NULL(transformComponent, ArgusECSLog);

	m_worldSpaceLocation = transformComponent->m_location;
	m_entityId = entityToRepresent.GetId();
	m_radius = transformComponent->m_radius;
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

bool ArgusEntityKDTreeNode::ShouldSkipNode(TFunction<bool(const ArgusEntityKDTreeNode* )> queryFilter) const
{
	if (m_entityId == ArgusECSConstants::k_maxEntities)
	{
		return true;
	}

	if (!queryFilter)
	{
		return false;
	}

	return !queryFilter(this);
}

bool ArgusEntityKDTreeNode::PassesRangeCheck(const FVector& targetLocation, float rangeSquared, float& nodeRangeSquared) const
{
	nodeRangeSquared = FVector::DistSquared2D(GetLocation(), targetLocation) - (m_radius * m_radius);
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

	if (thresholds.m_seenByEntityId != ArgusECSConstants::k_maxEntities)
	{
		ArgusEntity nodeToAddEntity = ArgusEntity::RetrieveEntity(nodeToAdd->m_entityId);
		ArgusEntity seenByEntity = ArgusEntity::RetrieveEntity(thresholds.m_seenByEntityId);
		const IdentityComponent* queryIdentityComponent = seenByEntity.GetComponent<IdentityComponent>();
		IdentityComponent* nodeToAddIdentityComponent = nodeToAddEntity.GetComponent<IdentityComponent>();
		if (queryIdentityComponent && nodeToAddIdentityComponent && seenByEntity.IsAlive())
		{
			nodeToAddIdentityComponent->AddSeenBy(queryIdentityComponent->m_team);
		}

		if (!nodeToAddEntity.IsAlive())
		{
			return;
		}
	}

	const uint16 entityId = nodeToAdd->m_entityId;
	if (distFromTargetSquared < thresholds.m_avoidanceRangeThresholdSquared)
	{
		m_entityIdsWithinAvoidanceRange.Add(entityId);
	}
	if (distFromTargetSquared < thresholds.m_flockingRangeThresholdSquared)
	{
		m_entityIdsWithinFlockingRange.Add(entityId);
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

void ArgusEntityKDTreeRangeOutput::ConsolidateInArray(TArray<uint16>& allEntities)
{
	const int32 totalSize = m_entityIdsWithinSightRange.Num() + m_entityIdsWithinRangedRange.Num() + m_entityIdsWithinMeleeRange.Num();
	allEntities.SetNumZeroed(totalSize);
	int32 counter = 0u;
	for (int32 i = 0; i < m_entityIdsWithinMeleeRange.Num(); ++i)
	{
		allEntities[counter] = m_entityIdsWithinMeleeRange[i];
		counter++;
	}
	for (int32 i = 0; i < m_entityIdsWithinRangedRange.Num(); ++i)
	{
		allEntities[counter] = m_entityIdsWithinRangedRange[i];
		counter++;
	}
	for (int32 i = 0; i < m_entityIdsWithinSightRange.Num(); ++i)
	{
		allEntities[counter] = m_entityIdsWithinSightRange[i];
		counter++;
	}
}

void ArgusEntityKDTreeRangeOutput::ResetAll()
{
	m_entityIdsWithinAvoidanceRange.Reset();
	m_entityIdsWithinMeleeRange.Reset();
	m_entityIdsWithinRangedRange.Reset();
	m_entityIdsWithinSightRange.Reset();
	m_entityIdsWithinFlockingRange.Reset();
}

bool ArgusEntityKDTreeRangeOutput::FoundAny() const
{
	return (m_entityIdsWithinSightRange.Num() + m_entityIdsWithinRangedRange.Num() + m_entityIdsWithinMeleeRange.Num()) > 0;
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

void ArgusEntityKDTree::SeedTreeWithAverageEntityLocation(bool forFlyingEntities)
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

		if (forFlyingEntities != retrievedEntity.IsFlying())
		{
			continue;
		}

		averageLocation += transformComponent->m_location;
		numIncludedEntities += 1.0f;
	}

	if (numIncludedEntities == 0.0f)
	{
		averageLocation = FVector::ZeroVector;
	}
	else
	{
		averageLocation = ArgusMath::SafeDivide(averageLocation, numIncludedEntities);
	}

	if (m_rootNode)
	{
		m_nodePool.Release(m_rootNode);
	}
	m_rootNode = m_nodePool.Take();
	m_rootNode->Populate(averageLocation);
}

void ArgusEntityKDTree::InsertAllArgusEntitiesIntoKDTree(bool forFlyingEntities)
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

		if (forFlyingEntities != retrievedEntity.IsFlying())
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
	ARGUS_RETURN_ON_NULL(transformComponent, ArgusECSLog);

	ArgusEntityKDTreeNode* nodeToInsert = m_nodePool.Take();
	nodeToInsert->Populate(entityToRepresent);
	if (!m_rootNode)
	{
		m_rootNode = nodeToInsert;
		return;
	}

	InsertNodeIntoKDTreeRecursive(m_rootNode, nodeToInsert, 0u);
}

bool ArgusEntityKDTree::RemoveArgusEntityFromKDTree(const ArgusEntity& entityToRemove)
{
	if (UNLIKELY(!entityToRemove))
	{
		return false;
	}

	ArgusEntityKDTreeNode* foundNode = nullptr;
	ArgusEntityKDTreeNode* parentNode = nullptr;
	if (!SearchForEntityIdRecursive(m_rootNode, entityToRemove.GetId(), foundNode, parentNode))
	{
		return false;
	}
	ARGUS_RETURN_ON_NULL_BOOL(foundNode, ArgusECSLog);

	if (parentNode->m_leftChild == foundNode)
	{
		parentNode->m_leftChild = nullptr;
	}
	else if (parentNode->m_rightChild == foundNode)
	{
		parentNode->m_rightChild = nullptr;
	}

	ArgusEntityKDTreeNode* leftChild = foundNode->m_leftChild;
	ArgusEntityKDTreeNode* rightChild = foundNode->m_rightChild;
	m_nodePool.Release(foundNode);
	RebuildSubTreeForArgusEntitiesRecursive(leftChild, true);
	RebuildSubTreeForArgusEntitiesRecursive(rightChild, true);
	return true;
}

uint16 ArgusEntityKDTree::FindArgusEntityIdClosestToLocation(const FVector& location) const
{
	return FindArgusEntityIdClosestToLocation(location, ArgusEntity::k_emptyEntity);
}

uint16 ArgusEntityKDTree::FindArgusEntityIdClosestToLocation(const FVector& location, const ArgusEntity& entityToIgnore) const
{
	const uint16 entityIdToIgnore = entityToIgnore.GetId();
	TFunction<bool(const ArgusEntityKDTreeNode*)> predicate = nullptr;
	if (entityIdToIgnore != ArgusECSConstants::k_maxEntities)
	{
		predicate = [entityIdToIgnore](const ArgusEntityKDTreeNode* nodeToSkip) 
		{ 
			if (!nodeToSkip)
			{
				return false;
			}
			return nodeToSkip->m_entityId != entityIdToIgnore;
		};
	}

	return FindArgusEntityIdClosestToLocation(location, predicate);
}

uint16 ArgusEntityKDTree::FindArgusEntityIdClosestToLocation(const FVector& location, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter) const
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

uint16 ArgusEntityKDTree::FindOtherArgusEntityIdClosestToArgusEntity(const ArgusEntity& entityToSearchAround, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride) const
{
	if (!entityToSearchAround)
	{
		ErrorOnInvalidArgusEntity(ARGUS_FUNCNAME);
		return ArgusECSConstants::k_maxEntities;
	}

	const TransformComponent* transformComponent = entityToSearchAround.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		ARGUS_ERROR_NULL(ArgusECSLog, transformComponent);
		return ArgusECSConstants::k_maxEntities;
	}

	if (queryFilterOverride)
	{
		return FindArgusEntityIdClosestToLocation(transformComponent->m_location, queryFilterOverride);
	}

	return FindArgusEntityIdClosestToLocation(transformComponent->m_location, entityToSearchAround);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range)
{
	return FindArgusEntityIdsWithinRangeOfLocation(outNearbyArgusEntityIds, location, range, ArgusEntity::k_emptyEntity);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const ArgusEntity& entityToIgnore)
{
	m_queryScratchData.ResetAll();
	FindArgusEntityIdsWithinRangeOfLocation(m_queryScratchData, ArgusEntityKDTreeQueryRangeThresholds(0.0f, 0.0f, range, 0.0f, ArgusECSConstants::k_maxEntities), location, range, entityToIgnore);
	m_queryScratchData.ConsolidateInArray(outNearbyArgusEntityIds);

	return outNearbyArgusEntityIds.Num() > 0u;
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(ArgusEntityKDTreeRangeOutput& output, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, const FVector& location, const float range, const ArgusEntity& entityToIgnore) const
{
	uint16 entityIdToIgnore = entityToIgnore.GetId();
	TFunction<bool(const ArgusEntityKDTreeNode*)> predicate = nullptr;
	if (entityIdToIgnore != ArgusECSConstants::k_maxEntities)
	{
		predicate = [entityIdToIgnore](const ArgusEntityKDTreeNode* nodeToSkip)
			{
				if (!nodeToSkip)
				{
					return false;
				}
				return nodeToSkip->m_entityId != entityIdToIgnore;
			};
	}

	return FindArgusEntityIdsWithinRangeOfLocation(output, thresholds, location, range, predicate);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(TArray<uint16>& outNearbyArgusEntityIds, const FVector& location, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride)
{
	m_queryScratchData.ResetAll();
	FindArgusEntityIdsWithinRangeOfLocation(m_queryScratchData, ArgusEntityKDTreeQueryRangeThresholds(0.0f, 0.0f, range, 0.0f, ArgusECSConstants::k_maxEntities), location, range, queryFilterOverride);
	m_queryScratchData.ConsolidateInArray(outNearbyArgusEntityIds);

	return outNearbyArgusEntityIds.Num() > 0u;
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinRangeOfLocation(ArgusEntityKDTreeRangeOutput& output, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, const FVector& location, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride) const
{
	ARGUS_RETURN_ON_NULL_BOOL(m_rootNode, ArgusECSLog);

	if (range <= 0.0f)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Searching range is less than or equal to 0."), ARGUS_FUNCNAME);
		return false;
	}

	FindNodesWithinRangeOfLocationRecursive(output, thresholds, m_rootNode, location, FMath::Square(range), queryFilterOverride, 0u);

	return output.FoundAny();
}

bool ArgusEntityKDTree::FindOtherArgusEntityIdsWithinRangeOfArgusEntity(TArray<uint16>& outNearbyArgusEntityIds, const ArgusEntity& entityToSearchAround, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride)
{
	m_queryScratchData.ResetAll();
	FindOtherArgusEntityIdsWithinRangeOfArgusEntity(m_queryScratchData, ArgusEntityKDTreeQueryRangeThresholds(0.0f, 0.0f, range, 0.0f, ArgusECSConstants::k_maxEntities), entityToSearchAround, range, queryFilterOverride);
	m_queryScratchData.ConsolidateInArray(outNearbyArgusEntityIds);

	return outNearbyArgusEntityIds.Num() > 0u;
}

bool ArgusEntityKDTree::FindOtherArgusEntityIdsWithinRangeOfArgusEntity(ArgusEntityKDTreeRangeOutput& output, const ArgusEntityKDTreeQueryRangeThresholds& thresholds, const ArgusEntity& entityToSearchAround, const float range, const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilterOverride) const
{
	if (!entityToSearchAround)
	{
		ErrorOnInvalidArgusEntity(ARGUS_FUNCNAME);
		return false;
	}

	const TransformComponent* transformComponent = entityToSearchAround.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(transformComponent, ArgusECSLog);

	if (queryFilterOverride)
	{
		return FindArgusEntityIdsWithinRangeOfLocation(output, thresholds, transformComponent->m_location, range, queryFilterOverride);
	}

	return FindArgusEntityIdsWithinRangeOfLocation(output, thresholds, transformComponent->m_location, range, entityToSearchAround);
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector>& convexPolygonPoints)
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

	m_queryScratchData.ResetAll();
	FindNodesWithinConvexPolyRecursive(m_queryScratchData, ArgusEntityKDTreeQueryRangeThresholds(0.0f, 0.0f, 0.0f, 0.0f, ArgusECSConstants::k_maxEntities), m_rootNode, convexPolygonPoints, nullptr, 0u);
	m_queryScratchData.ConsolidateInArray(outNearbyArgusEntityIds);

	return outNearbyArgusEntityIds.Num() > 0;
}

bool ArgusEntityKDTree::FindArgusEntityIdsWithinConvexPoly(TArray<uint16>& outNearbyArgusEntityIds, const TArray<FVector2D>& convexPolygonPoints)
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

void ArgusEntityKDTree::RequestInsertArgusEntityIntoKDTree(const ArgusEntity& entityToInsert)
{
	m_entityIdsToInsert.Add(entityToInsert.GetId());
}

void ArgusEntityKDTree::RequestRemoveArgusEntityIntoKDTree(const ArgusEntity& entityToRemove)
{
	m_entityIdsToRemove.Add(entityToRemove.GetId());
}

void ArgusEntityKDTree::ProcessDeferredStateChanges()
{
	for (int32 i = 0; i < m_entityIdsToRemove.Num(); ++i)
	{
		RemoveArgusEntityFromKDTree(ArgusEntity::RetrieveEntity(m_entityIdsToRemove[i]));
	}
	m_entityIdsToRemove.Reset();

	for (int32 i = 0; i < m_entityIdsToInsert.Num(); ++i)
	{
		InsertArgusEntityIntoKDTree(ArgusEntity::RetrieveEntity(m_entityIdsToInsert[i]));
	}
	m_entityIdsToInsert.Reset();
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

bool ArgusEntityKDTree::SearchForEntityIdRecursive(ArgusEntityKDTreeNode* node, uint16 entityId, ArgusEntityKDTreeNode*& outputNode, ArgusEntityKDTreeNode*& ouputParentNode)
{
	if (!node)
	{
		return false;
	}

	if (node->m_entityId == entityId)
	{
		outputNode = node;
		return true;
	}

	if (node->m_leftChild && SearchForEntityIdRecursive(node->m_leftChild, entityId, outputNode, ouputParentNode))
	{
		if (ouputParentNode == nullptr)
		{
			ouputParentNode = node;
		}
		return true;
	}

	if (node->m_rightChild && SearchForEntityIdRecursive(node->m_rightChild, entityId, outputNode, ouputParentNode))
	{
		if (ouputParentNode == nullptr)
		{
			ouputParentNode = node;
		}
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
	ARGUS_RETURN_ON_NULL(node, ArgusECSLog);
	const ArgusEntity entity = ArgusEntity::RetrieveEntity(node->m_entityId);
	if (UNLIKELY(!entity))
	{
		return;
	}

	m_nodePool.Release(node);
	InsertArgusEntityIntoKDTree(entity);
}

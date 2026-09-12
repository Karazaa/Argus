// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpatialPartitioningSystems.h"
#include "ArgusDetourQuery.h"
#include "ArgusECSConstants.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMath.h"
#include "ComponentDependencies/ObstaclePoint.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastHelpers.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/RecastQueryFilter.h"
#include "Systems/AvoidanceSystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusCVars.h"
#include "DrawDebugHelpers.h"
#endif //!UE_BUILD_SHIPPING

void SpatialPartitioningSystems::RunSystems()
{
	ARGUS_TRACE(SpatialPartitioningSystems::RunSystems);

	ArgusEntity spatialPartitioningEntity = ArgusEntity::GetSingletonEntity();
	if (!spatialPartitioningEntity)
	{
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = spatialPartitioningEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	spatialPartitioningComponent->m_argusEntityKDTree.ProcessDeferredStateChanges();
	spatialPartitioningComponent->m_flyingArgusEntityKDTree.ProcessDeferredStateChanges();
	spatialPartitioningComponent->m_argusEntityKDTree.RebuildKDTreeForAllArgusEntities();
	spatialPartitioningComponent->m_flyingArgusEntityKDTree.RebuildKDTreeForAllArgusEntities();

	ClearSeenByStatus();
	CacheAdjacentEntityIds(spatialPartitioningComponent);
	CalculateAdjacentEntityGroups();
}

void SpatialPartitioningSystems::GatherAvoidanceObstacles(UWorld* worldPointer, const FVector& queryOrigin, float queryExtent, FObstaclesContainer& outObstacles)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	const UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer);
	ARGUS_RETURN_ON_NULL(unrealNavigationSystem, ArgusECSLog);

	const ARecastNavMesh* navMesh = Cast<ARecastNavMesh>(unrealNavigationSystem->GetDefaultNavDataInstance());
	ARGUS_RETURN_ON_NULL(navMesh, ArgusECSLog);

	FNavLocation originLocation;
	if (!unrealNavigationSystem->ProjectPointToNavigation(queryOrigin, originLocation))
	{
		return;
	}

	TArray<FVector> navWalls;
	GetNavMeshWalls(queryExtent, navMesh, originLocation, navWalls);
	ConvertWallsIntoObstacles(navWalls, outObstacles);

#if !UE_BUILD_SHIPPING
	DrawDebugObstacles(worldPointer, outObstacles);
#endif //!UE_BUILD_SHIPPING
}

void SpatialPartitioningSystems::InitializeAvoidanceObstacles(SpatialPartitioningComponent* spatialPartitioningComponent, UWorld* worldPointer)
{
	ARGUS_TRACE(SpatialPartitioningSystems::InitializeAvoidanceObstacles);

	if (!spatialPartitioningComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in %s is nullptr."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpatialPartitioningComponent*));
		return;
	}

	spatialPartitioningComponent->m_obstacles.m_obstacleArrays.Reset();
	spatialPartitioningComponent->m_obstaclePointKDTree.ResetKDTreeWithAverageLocation();

	GatherAvoidanceObstacles(worldPointer, FVector::ZeroVector, spatialPartitioningComponent->m_validSpaceExtent, spatialPartitioningComponent->m_obstacles);

	spatialPartitioningComponent->m_obstaclePointKDTree.InsertObstaclesIntoKDTree(spatialPartitioningComponent->m_obstacles);

	ArgusIterators::IterateEntities([spatialPartitioningComponent](ArgusEntity entity)
	{
		TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
		TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
		NearbyObstaclesComponent* nearbyObstaclesComponent = entity.GetComponent<NearbyObstaclesComponent>();
		if (!transformComponent || !targetingComponent || !nearbyObstaclesComponent)
		{
			return;
		}

		nearbyObstaclesComponent->m_obstacleIndicies.ResetAll();
		ObstaclePointKDTreeQueryRangeThresholds thresholds = ObstaclePointKDTreeQueryRangeThresholds(AvoidanceSystems::GetAvoidanceRange(entity, AvoidanceRange::Obstacle));
		spatialPartitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation(nearbyObstaclesComponent->m_obstacleIndicies, thresholds, ArgusMath::ToCartesianVector(transformComponent->m_location), targetingComponent->m_sightRange);
	});
}

void SpatialPartitioningSystems::ClearSeenByStatus()
{
	ARGUS_TRACE(SpatialPartitioningSystems::ClearSeenByStatus);

	ArgusIterators::IterateEntities([](ArgusEntity entity)
	{
		if (IdentityComponent* identityComponent = entity.GetComponent<IdentityComponent>())
		{
			identityComponent->ClearSeenBy();
		}
	});
}

void SpatialPartitioningSystems::CacheAdjacentEntityIds(const SpatialPartitioningComponent* spatialPartitioningComponent)
{
	ARGUS_TRACE(SpatialPartitioningSystems::CacheAdjacentEntityIds);

	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	ArgusIterators::IterateEntitiesParallel<16u>([spatialPartitioningComponent](ArgusEntity entity)
	{
		NearbyEntitiesComponent* nearbyEntitiesComponent = entity.GetComponent<NearbyEntitiesComponent>();
		const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
		if (!nearbyEntitiesComponent || !transformComponent)
		{
			return;
		}

		nearbyEntitiesComponent->m_nearbyEntities.ResetAll();
		nearbyEntitiesComponent->m_nearbyFlyingEntities.ResetAll();

		if (AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>())
		{
			avoidanceGroupingComponent->m_groupId = ArgusECSConstants::k_maxEntities;
			avoidanceGroupingComponent->m_groupAverageLocation = FVector::ZeroVector;
			avoidanceGroupingComponent->m_numberOfIdleEntities = 0u;
			avoidanceGroupingComponent->m_entityIdsInGroup.Reset();
		}

		const float adjacentEntityRange = AvoidanceSystems::GetAvoidanceRange(entity, AvoidanceRange::Entity);
		const float groupExitRange = AvoidanceSystems::GetAvoidanceRange(entity, AvoidanceRange::GroupExit);

		const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter = [entity](const ArgusEntityKDTreeNode* entityNode)
		{
			ARGUS_RETURN_ON_NULL_BOOL(entityNode, ArgusECSLog);
			if (entityNode->m_entityId == entity.GetId())
			{
				return false;
			}

			ArgusEntity otherEntity = ArgusEntity::RetrieveEntity(entityNode->m_entityId);
			if (!otherEntity || otherEntity.IsPassenger())
			{
				return false;
			}

			return true;
		};

		float sightRange = adjacentEntityRange;
		if (const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>())
		{
			sightRange = targetingComponent->m_sightRange;
		}

		ArgusEntityKDTreeQueryRangeThresholds queryThresholds = ArgusEntityKDTreeQueryRangeThresholds(groupExitRange, adjacentEntityRange, transformComponent->m_radius, entity.GetId());
		spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(nearbyEntitiesComponent->m_nearbyEntities, queryThresholds, entity, sightRange, queryFilter);
		spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(nearbyEntitiesComponent->m_nearbyFlyingEntities, queryThresholds, entity, sightRange, queryFilter);
		
		if (transformComponent->m_movedThisFrame)
		{
			if (NearbyObstaclesComponent* nearbyObstaclesComponent = entity.GetComponent<NearbyObstaclesComponent>())
			{
				nearbyObstaclesComponent->m_obstacleIndicies.ResetAll();
				ObstaclePointKDTreeQueryRangeThresholds obstacleQueryThresholds = ObstaclePointKDTreeQueryRangeThresholds(AvoidanceSystems::GetAvoidanceRange(entity, AvoidanceRange::Obstacle));
				spatialPartitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation(nearbyObstaclesComponent->m_obstacleIndicies, obstacleQueryThresholds, ArgusMath::ToCartesianVector(transformComponent->m_location), sightRange);
			}
		}
	});
}

void SpatialPartitioningSystems::CalculateAdjacentEntityGroups()
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateAdjacentEntityGroups);

	ArgusIterators::IterateEntities([](ArgusEntity entity)
	{
		CalculateAdjacentEntityGroupsForEntity(entity, true);
	});
}

bool SpatialPartitioningSystems::FloodFillGroupRecursive(uint16 groupId, AvoidanceGroupingComponent* groupLeaderComponent, uint16 entityId, uint16 lastArgusEntityId, FVector& averageLocation, float& numberOfEntitiesInGroup, uint16& numberOfStoppedEntities)
{
	ARGUS_RETURN_ON_NULL_BOOL(groupLeaderComponent, ArgusECSLog);

	ArgusEntity entity = ArgusEntity::RetrieveEntity(entityId);
	ArgusEntity groupLeaderEntity = ArgusEntity::RetrieveEntity(groupId);
	ARGUS_RETURN_ON_INVALID_ENTITY_VALUE(entity, ArgusECSLog, false);
	ARGUS_RETURN_ON_INVALID_ENTITY_VALUE(groupLeaderEntity, ArgusECSLog, false);
	if (!entity.IsMoveable())
	{
		return false;
	}

	NearbyEntitiesComponent* nearbyEntitiesComponent = entity.GetComponent<NearbyEntitiesComponent>();
	AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>();
	const TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	const IdentityComponent* identityComponent = entity.GetComponent<IdentityComponent>();
	const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	const IdentityComponent* groupLeaderIdentityComponent = groupLeaderEntity.GetComponent<IdentityComponent>();
	const TargetingComponent* groupLeaderTargetingComponent = groupLeaderEntity.GetComponent<TargetingComponent>();
	if (!taskComponent || !nearbyEntitiesComponent || !avoidanceGroupingComponent || !transformComponent || !identityComponent || !targetingComponent || !groupLeaderIdentityComponent || !groupLeaderTargetingComponent)
	{
		return false;
	}

	if (avoidanceGroupingComponent->m_groupId != ArgusECSConstants::k_maxEntities)
	{
		return false;
	}

	if (identityComponent->m_team != groupLeaderIdentityComponent->m_team)
	{
		return false;
	}

	if (!targetingComponent->HasSameTarget(groupLeaderTargetingComponent))
	{
		return false;
	}

	// If it's the same group, we don't need to do the range constraint
	if (groupId != avoidanceGroupingComponent->m_previousGroupId)
	{
		const float range = AvoidanceSystems::GetAvoidanceRange(entity, AvoidanceRange::GroupEnter);
		if (!entity.IsInRangeOfOtherEntity(ArgusEntity::RetrieveEntity(lastArgusEntityId), range))
		{
			return false;
		}
	}

	avoidanceGroupingComponent->m_groupId = groupId;
	averageLocation += transformComponent->m_location;
	numberOfEntitiesInGroup += 1.0f;
	groupLeaderComponent->m_entityIdsInGroup.Add(entity.GetId());
	if (entity.IsIdle())
	{
		numberOfStoppedEntities++;
	}

	const bool isGrounded = taskComponent->m_flightState == EFlightState::Grounded;
	for (int32 i = 0; i < nearbyEntitiesComponent->GetNearbyEntities(!isGrounded).GetEntityIdsInGroupExitRange().Num(); ++i)
	{
		FloodFillGroupRecursive(groupId, groupLeaderComponent, nearbyEntitiesComponent->GetNearbyEntities(!isGrounded).GetEntityIdsInGroupExitRange()[i], entityId, averageLocation, numberOfEntitiesInGroup, numberOfStoppedEntities);
	}

	return groupId == entityId;
}

void SpatialPartitioningSystems::OnBecomeAvoidanceGroupLeader(ArgusEntity entity)
{
	ARGUS_RETURN_ON_INVALID_ENTITY(entity, ArgusECSLog);

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	ARGUS_RETURN_ON_NULL(taskComponent, ArgusECSLog);

	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	ARGUS_RETURN_ON_NULL(targetingComponent, ArgusECSLog);

	if (targetingComponent->HasEntityTarget())
	{
		taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	}
	else if (targetingComponent->HasLocationTarget())
	{
		taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
	}
}

void SpatialPartitioningSystems::OnChangeAvoidanceGroups(ArgusEntity entity, AvoidanceGroupingComponent* groupingComponent)
{
	ARGUS_RETURN_ON_INVALID_ENTITY(entity, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(groupingComponent, ArgusECSLog);

	NavigationComponent* navigationComponent = entity.GetComponent<NavigationComponent>();
	ARGUS_RETURN_ON_NULL(navigationComponent, ArgusECSLog);

	ArgusEntity groupLeaderEntity = ArgusEntity::RetrieveEntity(groupingComponent->m_groupId);
	ARGUS_RETURN_ON_INVALID_ENTITY(groupLeaderEntity, ArgusECSLog);

	NavigationComponent* groupLeaderNavigationComponent = groupLeaderEntity.GetComponent<NavigationComponent>();
	ARGUS_RETURN_ON_NULL(groupLeaderNavigationComponent, ArgusECSLog);

	navigationComponent->m_lastPointIndex = groupLeaderNavigationComponent->m_groupLastPointIndex;
}

float SpatialPartitioningSystems::FindAreaOfObstacleCartesian(const FObstaclePointArray& obstaclePoints)
{
	float area = 0.0f;

	for (int32 i = 0; i < obstaclePoints.m_obstaclePoints.Num(); ++i)
	{
		FVector2D point0 = obstaclePoints.m_obstaclePoints[i].m_point;
		FVector2D point1 = obstaclePoints.m_obstaclePoints[(i + 1) % obstaclePoints.m_obstaclePoints.Num()].m_point;

		float width = point1.X - point0.X;
		float height = (point1.Y + point0.Y) / 2.0f;

		area += width * height;
	}

	return area;
}

bool SpatialPartitioningSystems::IsEntityInLineOfSightOfOther(ArgusEntity sourceEntity, ArgusEntity targetEntity)
{
	if (!sourceEntity || !targetEntity)
	{
		return false;
	}

	const TransformComponent* targetTransformComponent = targetEntity.GetComponent<TransformComponent>();
	if (!targetTransformComponent)
	{
		return false;
	}

	return IsPointInLineOfSightOfEntity(sourceEntity, targetTransformComponent->m_location);
}

bool SpatialPartitioningSystems::IsPointInLineOfSightOfEntity(ArgusEntity sourceEntity, const FVector& targetLocation)
{
	ARGUS_TRACE(SpatialPartitioningSystems::IsPointInLineOfSightOfEntity);

	if (!sourceEntity)
	{
		return false;
	}

	const TransformComponent* transformComponent = sourceEntity.GetComponent<TransformComponent>();
	const NearbyObstaclesComponent* nearbyObstaclesComponent = sourceEntity.GetComponent<NearbyObstaclesComponent>();
	const TargetingComponent* targetingComponent = sourceEntity.GetComponent<TargetingComponent>();
	if (!transformComponent || !nearbyObstaclesComponent || !targetingComponent)
	{
		return false;
	}

	if (FVector::DistSquared2D(transformComponent->m_location, targetLocation) > FMath::Square(targetingComponent->m_sightRange))
	{
		return false;
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	const FogOfWarComponent* fogOfWarComponent = ArgusEntity::GetSingletonEntity().GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(spatialPartitioningComponent, ArgusECSLog);

	const FVector2D cartesianSourceLocation = FVector2D(ArgusMath::ToCartesianVector(transformComponent->m_location));
	const FVector2D cartesianTargetLocation = FVector2D(ArgusMath::ToCartesianVector(targetLocation));

	bool returnValue = true;
	nearbyObstaclesComponent->m_obstacleIndicies.IterateObstacleIndiciesInSightRange([spatialPartitioningComponent, fogOfWarComponent, &cartesianSourceLocation, &cartesianTargetLocation, &returnValue](ObstacleIndicies indicies)
	{
		if (spatialPartitioningComponent->IsPointElevated(indicies) || spatialPartitioningComponent->IsNextPointElevated(indicies))
		{
			return;
		}

		const FObstaclePoint& currentObstaclePoint = spatialPartitioningComponent->GetObstaclePointFromIndicies(indicies);
		const FObstaclePoint& nextObstaclePoint = spatialPartitioningComponent->GetNextObstaclePointFromIndicies(indicies);

		FVector2D currentPoint = currentObstaclePoint.m_point;
		const FVector2D currentLeft = currentObstaclePoint.GetLeftVector();
		FVector2D nextPoint = nextObstaclePoint.m_point;
		const FVector2D nextLeft = nextObstaclePoint.GetLeftVector();

		currentPoint += (currentLeft * fogOfWarComponent->m_visionObstacleAdjustDistance);
		nextPoint += (nextLeft * (fogOfWarComponent ? fogOfWarComponent->m_visionObstacleAdjustDistance : 0.0f));

		if (ArgusMath::IsLeftOfCartesian(cartesianSourceLocation, currentPoint, nextPoint))
		{
			return;
		}

		if (ArgusMath::DoLineSegmentsIntersectCartesian(cartesianSourceLocation, cartesianTargetLocation, currentPoint, nextPoint))
		{
			returnValue = false;
			return;
		}
	});

	return returnValue;
}

bool SpatialPartitioningSystems::AnyObstaclesOrStaticEntitiesInCircle(const FVector& center, float radius, float resourceSourceBufferRadius)
{
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(spatialPartitioningComponent, ArgusInputLog);

	TArray<ObstacleIndicies> obstacleIndicies;
	FVector location = ArgusMath::ToCartesianVector(center);
	location.Z = 0.0f;
	spatialPartitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation(obstacleIndicies, location, radius);
	bool anyFound = obstacleIndicies.Num() > 0;

	if (!anyFound)
	{
		TArray<uint16> nearbyArgusEntityIds;
		spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(nearbyArgusEntityIds, center, radius + resourceSourceBufferRadius, [&center, radius, resourceSourceBufferRadius](const ArgusEntityKDTreeNode* node)
		{
			if (!node)
			{
				return false;
			}

			ArgusEntity entity = ArgusEntity::RetrieveEntity(node->m_entityId);
			if (!entity)
			{
				return false;
			}

			if (entity.IsMoveable())
			{
				return false;
			}

			const float distance = FVector::Dist2D(node->m_worldSpaceLocation, center) - node->m_radius;
			if (distance < radius)
			{
				return true;
			}

			ResourceComponent* resourceComponent = entity.GetComponent<ResourceComponent>();
			if (!resourceComponent)
			{
				return false;
			}

			return (resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Source) && (distance < resourceSourceBufferRadius);
		});
		anyFound = nearbyArgusEntityIds.Num() > 0;
	}

	return anyFound;
}

void SpatialPartitioningSystems::CalculateAdjacentEntityGroupsForEntity(ArgusEntity entity, bool allowNavigationRecalculation)
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateAdjacentEntityGroupsForEntity);
	ARGUS_RETURN_ON_INVALID_ENTITY(entity, ArgusECSLog);

	FVector averageLocation = FVector::ZeroVector;
	float numberOfEntitiesInGroup = 0.0f;
	uint16 numberOfStoppedEntities = 0u;
	AvoidanceGroupingComponent* groupLeaderComponent = entity.GetComponent<AvoidanceGroupingComponent>();
	if (!groupLeaderComponent)
	{
		return;
	}

	if (FloodFillGroupRecursive(entity.GetId(), groupLeaderComponent, entity.GetId(), entity.GetId(), averageLocation, numberOfEntitiesInGroup, numberOfStoppedEntities))
	{
		groupLeaderComponent->m_groupAverageLocation = ArgusMath::SafeDivide(averageLocation, numberOfEntitiesInGroup);
		groupLeaderComponent->m_numberOfIdleEntities = numberOfStoppedEntities;
	}

	if (!allowNavigationRecalculation)
	{
		groupLeaderComponent->m_previousGroupId = groupLeaderComponent->m_groupId;
		return;
	}

	if (groupLeaderComponent->m_previousGroupId == groupLeaderComponent->m_groupId)
	{
		return;
	}

	if (groupLeaderComponent->m_previousGroupId == ArgusECSConstants::k_maxEntities || groupLeaderComponent->m_groupId == ArgusECSConstants::k_maxEntities)
	{
		groupLeaderComponent->m_previousGroupId = groupLeaderComponent->m_groupId;
		return;
	}

	if (groupLeaderComponent->m_groupId == entity.GetId())
	{
		OnBecomeAvoidanceGroupLeader(entity);
	}
	else
	{
		OnChangeAvoidanceGroups(entity, groupLeaderComponent);
	}

	groupLeaderComponent->m_previousGroupId = groupLeaderComponent->m_groupId;
}

bool SpatialPartitioningSystems::GetNavMeshWalls(float queryExtent, const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls)
{
	ARGUS_TRACE(SpatialPartitioningSystems::GetNavMeshWalls);

	ARGUS_RETURN_ON_NULL_BOOL(navMesh, ArgusECSLog);

	const FNavigationQueryFilter* filter = navMesh->GetDefaultQueryFilter().Get();
	ARGUS_RETURN_ON_NULL_BOOL(filter, ArgusECSLog);

	const dtNavMesh* detourMesh = navMesh->GetRecastMesh();
	ARGUS_RETURN_ON_NULL_BOOL(detourMesh, ArgusECSLog);

	const uint32 maxSearchNodes = filter->GetMaxSearchNodes();
	const FRecastQueryFilter* recastQueryFilter = static_cast<const FRecastQueryFilter*>(filter->GetImplementation());
	ARGUS_RETURN_ON_NULL_BOOL(recastQueryFilter, ArgusECSLog);

	const dtQueryFilter* queryFilter = recastQueryFilter->GetAsDetourQueryFilter();
	ARGUS_RETURN_ON_NULL_BOOL(queryFilter, ArgusECSLog);

	int32 numWalls = 0;
	FVector::FReal wallSegments[ArgusECSConstants::k_maxDetourWalls * 3 * 2] = { 0 };
	dtPolyRef wallPolys[ArgusECSConstants::k_maxDetourWalls * 2] = { 0 };

	int32 numNeis = 0;
	dtPolyRef neiPolys[ArgusECSConstants::k_maxDetourPolys] = { 0 };

	const int verts = 4;
	TArray<FVector> queryShapePoints;
	queryShapePoints.SetNumZeroed(verts);
	queryShapePoints[0].X -= queryExtent;
	queryShapePoints[1].X += queryExtent;
	queryShapePoints[2].X += queryExtent;
	queryShapePoints[3].X -= queryExtent;
	queryShapePoints[0].Y += queryExtent;
	queryShapePoints[1].Y += queryExtent;
	queryShapePoints[2].Y -= queryExtent;
	queryShapePoints[3].Y -= queryExtent;

	FVector::FReal rcConvexPolygon[verts * 3] = { 0 };

	for (int32 i = 0; i < verts; i++)
	{
		const FVector RcPoint = Unreal2RecastPoint(queryShapePoints[i]);
		rcConvexPolygon[i * 3 + 0] = RcPoint.X;
		rcConvexPolygon[i * 3 + 1] = RcPoint.Y;
		rcConvexPolygon[i * 3 + 2] = RcPoint.Z;
	}

	dtStatus queryStatus = ArgusDetourQuery::FindWallsOverlappingShape
	(
		detourMesh, maxSearchNodes, originLocation.NodeRef, rcConvexPolygon, verts, queryFilter,
		neiPolys, &numNeis, ArgusECSConstants::k_maxDetourPolys, wallSegments, wallPolys, &numWalls, ArgusECSConstants::k_maxDetourWalls
	);

	if (dtStatusSucceed(queryStatus))
	{
		outNavWalls.Reset(numWalls * 2);
		for (int32 Idx = 0; Idx < numWalls; Idx++)
		{
			FVector vertex0 = Recast2UnrealPoint(&wallSegments[Idx * 6]);
			FVector vertex1 = Recast2UnrealPoint(&wallSegments[Idx * 6 + 3]);

			const bool excluded =	((vertex0.X < -queryExtent || vertex0.X > queryExtent) ||
									(vertex0.Y < -queryExtent || vertex0.Y > queryExtent)) &&
									((vertex1.X < -queryExtent || vertex1.X > queryExtent) ||
									(vertex1.Y < -queryExtent || vertex1.Y > queryExtent));
			if (!excluded)
			{
				outNavWalls.Add(vertex0);
				outNavWalls.Add(vertex1);
			}
		}

		return true;
	}

	return false;
}

void SpatialPartitioningSystems::ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, FObstaclesContainer& outObstacles)
{
	ARGUS_TRACE(SpatialPartitioningSystems::ConvertWallsIntoObstacles);

	const int32 numNavEdges = navEdges.Num();
	if ((numNavEdges % 2) != 0 || numNavEdges == 0)
	{
		return;
	}

	for (int32 i = 0; i < numNavEdges; i += 2)
	{
		const float edgeVertex0Height = navEdges[i].Z;
		const float edgeVertex1Height = navEdges[i + 1].Z;
		const FVector2D edgeVertex0 = ArgusMath::ToCartesianVector2(FVector2D(navEdges[i]));
		const FVector2D edgeVertex1 = ArgusMath::ToCartesianVector2(FVector2D(navEdges[i + 1]));

		bool handledEdge = false;
		for (int32 j = 0; j < outObstacles.m_obstacleArrays.Num(); ++j)
		{
			const int32 numObstaclesInChain = outObstacles.m_obstacleArrays[j].m_obstaclePoints.Num();
			if (numObstaclesInChain == 0)
			{
				continue;
			}

			const FVector2D startOfChainLocation = outObstacles.m_obstacleArrays[j].GetHead().m_point;
			const FVector2D endOfChainLocation = outObstacles.m_obstacleArrays[j].GetTail().m_point;
			FObstaclePoint pointToAdd;
			bool matchesStart = false;
			bool matchesEnd = false;
			if (startOfChainLocation == edgeVertex0)
			{
				matchesStart = true;
				pointToAdd.m_point = edgeVertex1;
				pointToAdd.m_height = edgeVertex1Height;
			}
			if (startOfChainLocation == edgeVertex1)
			{
				matchesStart = true;
				pointToAdd.m_point = edgeVertex0;
				pointToAdd.m_height = edgeVertex0Height;
			}
			if (endOfChainLocation == edgeVertex0)
			{
				matchesEnd = true;
				pointToAdd.m_point = edgeVertex1;
				pointToAdd.m_height = edgeVertex1Height;
			}
			if (endOfChainLocation == edgeVertex1)
			{
				matchesEnd = true;
				pointToAdd.m_point = edgeVertex0;
				pointToAdd.m_height = edgeVertex0Height;
			}

			if (matchesStart && !matchesEnd)
			{
				outObstacles.m_obstacleArrays[j].AddObstaclePointsWithFillIn(pointToAdd, true);
				handledEdge = true;
				break;
			}
			if (!matchesStart && matchesEnd)
			{
				outObstacles.m_obstacleArrays[j].AddObstaclePointsWithFillIn(pointToAdd, false);
				handledEdge = true;
				break;
			}
			if (matchesStart && matchesEnd)
			{
				handledEdge = true;
				break;
			}
		}

		if (handledEdge)
		{
			continue;
		}

		FObstaclePoint vertex0Obstacle, vertex1Obstacle;
		vertex0Obstacle.m_point = edgeVertex0;
		vertex0Obstacle.m_height = edgeVertex0Height;
		vertex1Obstacle.m_point = edgeVertex1;
		vertex1Obstacle.m_height = edgeVertex1Height;
		FObstaclePointArray& array = outObstacles.m_obstacleArrays.Emplace_GetRef();
		array.m_obstaclePoints.Add(vertex0Obstacle);
		array.AddObstaclePointsWithFillIn(vertex1Obstacle, false);
	}

	const int32 initialSize = outObstacles.m_obstacleArrays.Num();
	for (int32 i = (initialSize - 1); i >= 0; --i)
	{
		FObstaclePointArray& appending = outObstacles.m_obstacleArrays[i];
		FVector2D appendingObstacleHead = appending.GetHead().m_point;
		FVector2D appendingObstacleTail = appending.GetTail().m_point;

		bool didAppend = false;
		for (int32 j = i - 1; j >= 0; --j)
		{
			FObstaclePointArray& receiving = outObstacles.m_obstacleArrays[j];
			FVector2D receivingObstacleHead = receiving.GetHead().m_point;
			FVector2D receivingObstacleTail = receiving.GetTail().m_point;

			if (receivingObstacleTail == appendingObstacleHead)
			{
				// just append to receiving
				receiving.AppendOtherToThis(appending);
				didAppend = true;
				break;
			}
			else if (receivingObstacleHead == appendingObstacleTail)
			{
				// append to appending, then assign appending to receiving
				appending.AppendOtherToThis(receiving);
				receiving = appending;
				didAppend = true;
				break;
			}
			else if (receivingObstacleHead == appendingObstacleHead)
			{
				// reverse appending, then append to appending, then assign to receiving
				appending.Reverse();
				appending.AppendOtherToThis(receiving);
				receiving = appending;
				didAppend = true;
				break;
			}
			else if (receivingObstacleTail == appendingObstacleTail)
			{
				// reverse appending, then append to receiving
				appending.Reverse();
				receiving.AppendOtherToThis(appending);
				didAppend = true;
				break;
			}
		}

		if (didAppend)
		{
			outObstacles.m_obstacleArrays.RemoveAt(i, EAllowShrinking::No);
		}
	}

	outObstacles.m_obstacleArrays.Shrink();

	for (int32 i = 0; i < outObstacles.m_obstacleArrays.Num(); ++i)
	{
		outObstacles.m_obstacleArrays[i].CloseLoop();
		outObstacles.m_obstacleArrays[i].ConsolidateNearbyPoints();
		outObstacles.m_obstacleArrays[i].m_obstaclePoints.Shrink();
		outObstacles.m_obstacleArrays[i].m_fixupDirections.Reserve(outObstacles.m_obstacleArrays[i].m_obstaclePoints.Num());
		CalculateFixupDirectionForObstacles(outObstacles.m_obstacleArrays[i]);
		ApplyFixupDirectionForObstacles(outObstacles.m_obstacleArrays[i]);
		CalculateDirectionAndConvexForObstacles(outObstacles.m_obstacleArrays[i]);
		outObstacles.m_obstacleArrays[i].m_fixupDirections.Empty();
	}
}

void SpatialPartitioningSystems::CalculateFixupDirectionForObstacles(FObstaclePointArray& outObstacle)
{
	const int32 numObstaclePoints = outObstacle.m_obstaclePoints.Num();
	float fixupDirectionMult = -1.0f;
	if (FindAreaOfObstacleCartesian(outObstacle) > 0.0f)
	{
		fixupDirectionMult = 1.0f;
		outObstacle.Reverse();
	}

	float floorHeight = FLT_MAX;
	for (int32 i = 0; i < numObstaclePoints; ++i)
	{
		const int32 nextIndex = (i + 1) % numObstaclePoints;
		const int32 lastIndex = (i - 1) >= 0 ? (i - 1) : numObstaclePoints - 1;

		const FVector2D neighborDirection = outObstacle.m_obstaclePoints[nextIndex].m_point - outObstacle.m_obstaclePoints[lastIndex].m_point;
		outObstacle.m_fixupDirections.Add(FVector2D(-neighborDirection.Y, neighborDirection.X).GetSafeNormal() * fixupDirectionMult);
	}
}

void SpatialPartitioningSystems::ApplyFixupDirectionForObstacles(FObstaclePointArray& outObstacle)
{
	const GlobalSettingsComponent* settings = GlobalSettingsComponent::Get();
	ARGUS_RETURN_ON_NULL(settings, ArgusECSLog);

	for (int32 i = 0; i < outObstacle.m_obstaclePoints.Num(); ++i)
	{
		outObstacle.m_obstaclePoints[i].m_point += (outObstacle.m_fixupDirections[i] * settings->m_obstacleShrinkFixupDistance);
	}
}

void SpatialPartitioningSystems::CalculateDirectionAndConvexForObstacles(FObstaclePointArray& outObstacle)
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateDirectionAndConvexForObstacles);

	const int32 numObstaclePoints = outObstacle.m_obstaclePoints.Num();

	float floorHeight = FLT_MAX;
	for (int32 i = 0; i < numObstaclePoints; ++i)
	{
		const int32 nextIndex = (i + 1) % numObstaclePoints;
		outObstacle.m_obstaclePoints[i].m_direction = outObstacle.m_obstaclePoints[nextIndex].m_point - outObstacle.m_obstaclePoints[i].m_point;
		outObstacle.m_obstaclePoints[i].m_direction.Normalize();

		const int32 lastIndex = (i - 1) >= 0 ? (i - 1) : numObstaclePoints - 1;
		outObstacle.m_obstaclePoints[i].m_isConvex = ArgusMath::IsLeftOfCartesian(outObstacle.m_obstaclePoints[lastIndex].m_point, outObstacle.m_obstaclePoints[i].m_point, outObstacle.m_obstaclePoints[nextIndex].m_point);

		floorHeight = outObstacle.m_obstaclePoints[i].m_height < floorHeight ? outObstacle.m_obstaclePoints[i].m_height : floorHeight;
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);
	outObstacle.m_floorHeight = floorHeight + spatialPartitioningComponent->m_elevatedObstaclePointHeightThreshold;
}

#if !UE_BUILD_SHIPPING
void SpatialPartitioningSystems::DrawDebugObstacles(UWorld* worldPointer, const FObstaclesContainer& obstacles)
{
	if (!worldPointer)
	{
		return;
	}

	if (!ArgusCVars::CVarShowObstacleDebug.GetValueOnGameThread())
	{
		return;
	}

	const GlobalSettingsComponent* settings = GlobalSettingsComponent::Get();
	ARGUS_RETURN_ON_NULL(settings, ArgusECSLog);

	for (int32 i = 0; i < obstacles.m_obstacleArrays.Num(); ++i)
	{
		for (int32 j = 0; j < obstacles.m_obstacleArrays[i].m_obstaclePoints.Num(); ++j)
		{
			obstacles.m_obstacleArrays[i].m_obstaclePoints[j].DrawDebugObstaclePoint(worldPointer, 120.0f, false, obstacles.m_obstacleArrays[i].IsPointElevated(j));
		}
	}
}
#endif //!UE_BUILD_SHIPPING
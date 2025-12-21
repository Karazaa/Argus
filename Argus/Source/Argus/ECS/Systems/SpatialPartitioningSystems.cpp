// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpatialPartitioningSystems.h"
#include "ArgusDetourQuery.h"
#include "ArgusECSConstants.h"
#include "ArgusLogging.h"
#include "ArgusMath.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastHelpers.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/RecastQueryFilter.h"

static TAutoConsoleVariable<bool> CVarShowObstacleDebug(TEXT("Argus.SpatialPartitioning.ShowAvoidanceObstacleDebug"), false, TEXT(""));

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

void SpatialPartitioningSystems::ClearSeenByStatus()
{
	ARGUS_TRACE(SpatialPartitioningSystems::ClearSeenByStatus);

	ArgusEntity::IterateEntities([](ArgusEntity entity) 
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

	ArgusEntity::IterateEntities([spatialPartitioningComponent](ArgusEntity entity) 
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
		}

		float adjacentEntityRange = transformComponent->m_radius;
		if (const VelocityComponent* velocityComponent = entity.GetComponent<VelocityComponent>())
		{
			adjacentEntityRange += velocityComponent->m_desiredSpeedUnitsPerSecond;
		}

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

		float meleeRange = 0.0f;
		float rangedRange = 0.0f;
		float sightRange = adjacentEntityRange;
		if (const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>())
		{
			meleeRange = targetingComponent->m_meleeRange;
			rangedRange = targetingComponent->m_rangedRange;
			sightRange = targetingComponent->m_sightRange;
		}
		ArgusEntityKDTreeQueryRangeThresholds queryThresholds = ArgusEntityKDTreeQueryRangeThresholds(rangedRange, meleeRange, adjacentEntityRange, entity.GetId());
		spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(nearbyEntitiesComponent->m_nearbyEntities, queryThresholds, entity, sightRange, queryFilter);
		spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(nearbyEntitiesComponent->m_nearbyFlyingEntities, queryThresholds, entity, sightRange, queryFilter);
		if (NearbyObstaclesComponent* nearbyObstaclesComponent = entity.GetComponent<NearbyObstaclesComponent>())
		{
			// TODO JAMES: Gate updates by whether or not the entity is capable of moving?
			nearbyObstaclesComponent->m_obstacleIndicies.ResetAll();
			spatialPartitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation(nearbyObstaclesComponent->m_obstacleIndicies, ArgusMath::ToCartesianVector(transformComponent->m_location), sightRange);
		}
	});
}

void SpatialPartitioningSystems::CalculateAdjacentEntityGroups()
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateAdjacentEntityGroups);

	ArgusEntity::IterateEntities([](ArgusEntity entity) 
	{
		FVector averageLocation = FVector::ZeroVector;
		float numberOfEntitiesInGroup = 0.0f;
		uint16 numberOfStoppedEntities = 0u;
		TArray<ArgusEntity> entitiesInGroup;
		if (!FloodFillGroupRecursive(entity.GetId(), entity.GetId(), averageLocation, numberOfEntitiesInGroup, numberOfStoppedEntities, entitiesInGroup))
		{
			return;
		}

		AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>();
		if (!avoidanceGroupingComponent)
		{
			return;
		}

		avoidanceGroupingComponent->m_groupAverageLocation = ArgusMath::SafeDivide(averageLocation, numberOfEntitiesInGroup);
		avoidanceGroupingComponent->m_numberOfIdleEntities = numberOfStoppedEntities;

		for (int32 j = 0; j < entitiesInGroup.Num(); ++j)
		{
			const TaskComponent* taskComponent = entitiesInGroup[j].GetComponent<TaskComponent>();
			if (!taskComponent)
			{
				continue;
			}

			const TransformComponent* transformComponent = entitiesInGroup[j].GetComponent<TransformComponent>();
			if (!transformComponent)
			{
				continue;
			}
		}
	});
}

bool SpatialPartitioningSystems::FloodFillGroupRecursive(uint16 groupId, uint16 entityId, FVector& averageLocation, float& numberOfEntitiesInGroup, uint16& numberOfStoppedEntities, TArray<ArgusEntity>& entitiesInGroup)
{
	ArgusEntity entity = ArgusEntity::RetrieveEntity(entityId);
	ArgusEntity groupLeaderEntity = ArgusEntity::RetrieveEntity(groupId);
	if (!entity || !groupLeaderEntity || !entity.IsMoveable())
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

	avoidanceGroupingComponent->m_groupId = groupId;
	averageLocation += transformComponent->m_location;
	numberOfEntitiesInGroup += 1.0f;
	entitiesInGroup.Add(entity);
	if (entity.IsIdle())
	{
		numberOfStoppedEntities++;
	}

	const bool isGrounded = taskComponent->m_flightState == EFlightState::Grounded;
	for (int32 i = 0; i < nearbyEntitiesComponent->GetNearbyEntities(!isGrounded).GetEntityIdsInAvoidanceRange().Num(); ++i)
	{
		FloodFillGroupRecursive(groupId, nearbyEntitiesComponent->GetNearbyEntities(!isGrounded).GetEntityIdsInAvoidanceRange()[i], averageLocation, numberOfEntitiesInGroup, numberOfStoppedEntities, entitiesInGroup);
	}

	return groupId == entityId;
}

void SpatialPartitioningSystems::CalculateAvoidanceObstacles(SpatialPartitioningComponent* spatialPartitioningComponent, UWorld* worldPointer)
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateAvoidanceObstacles);

	if (!spatialPartitioningComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in %s is nullptr."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpatialPartitioningComponent*));
		return;
	}

	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in %s is nullptr."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	const UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer);
	if (!unrealNavigationSystem)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UNavigationSystemV1*));
		return;
	}

	const ARecastNavMesh* navMesh = Cast<ARecastNavMesh>(unrealNavigationSystem->MainNavData);
	if (!navMesh)
	{
		return;
	}

	FNavLocation originLocation;
	if (!unrealNavigationSystem->ProjectPointToNavigation(FVector::ZeroVector, originLocation))
	{
		return;
	}

	spatialPartitioningComponent->m_obstacles.Empty();
	spatialPartitioningComponent->m_obstaclePointKDTree.ResetKDTreeWithAverageLocation();

	TArray<FVector> navWalls;
	GetNavMeshWalls(spatialPartitioningComponent, navMesh, originLocation, navWalls);

	ConvertWallsIntoObstacles(navWalls, spatialPartitioningComponent->m_obstacles);

	DrawDebugObstacles(worldPointer, spatialPartitioningComponent->m_obstacles);

	spatialPartitioningComponent->m_obstaclePointKDTree.InsertObstaclesIntoKDTree(spatialPartitioningComponent->m_obstacles);

	ArgusEntity::IterateEntities([spatialPartitioningComponent](ArgusEntity entity)
	{
		TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
		TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
		NearbyObstaclesComponent* nearbyObstaclesComponent = entity.GetComponent<NearbyObstaclesComponent>();
		if (!transformComponent || !targetingComponent || !nearbyObstaclesComponent)
		{
			return;
		}

		nearbyObstaclesComponent->m_obstacleIndicies.ResetAll();
		spatialPartitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation(nearbyObstaclesComponent->m_obstacleIndicies, ArgusMath::ToCartesianVector(transformComponent->m_location), targetingComponent->m_sightRange);
	});
}

float SpatialPartitioningSystems::FindAreaOfObstacleCartesian(const ObstaclePointArray& obstaclePoints)
{
	float area = 0.0f;

	for (int32 i = 0; i < obstaclePoints.Num(); ++i)
	{
		FVector2D point0 = obstaclePoints[i].m_point;
		FVector2D point1 = obstaclePoints[(i + 1) % obstaclePoints.Num()].m_point;

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
	const TArray<ObstacleIndicies, ArgusContainerAllocator<20u> >& inRangeObstacleIndicies = nearbyObstaclesComponent->m_obstacleIndicies.GetInRangeObstacleIndicies();

	for (int32 i = 0; i < inRangeObstacleIndicies.Num(); ++i)
	{
		const ObstacleIndicies& obstacleIndicies = inRangeObstacleIndicies[i];
		if (spatialPartitioningComponent->IsPointElevated(obstacleIndicies) || spatialPartitioningComponent->IsNextPointElevated(obstacleIndicies))
		{
			continue;
		}

		const ObstaclePoint& currentObstaclePoint = spatialPartitioningComponent->GetObstaclePointFromIndicies(obstacleIndicies);
		const ObstaclePoint& nextObstaclePoint = spatialPartitioningComponent->GetNextObstaclePointFromIndicies(obstacleIndicies);

		FVector2D currentPoint = currentObstaclePoint.m_point;
		const FVector2D currentLeft = currentObstaclePoint.GetLeftVector();
		FVector2D nextPoint = nextObstaclePoint.m_point;
		const FVector2D nextLeft = nextObstaclePoint.GetLeftVector();

		currentPoint += (currentLeft * fogOfWarComponent->m_visionObstacleAdjustDistance);
		nextPoint += (nextLeft * (fogOfWarComponent ? fogOfWarComponent->m_visionObstacleAdjustDistance : 0.0f));

		if (ArgusMath::IsLeftOfCartesian(cartesianSourceLocation, currentPoint, nextPoint))
		{
			continue;
		}

		if (ArgusMath::DoLineSegmentsIntersectCartesian(cartesianSourceLocation, cartesianTargetLocation, currentPoint, nextPoint))
		{
			return false;
		}
	}

	return true;
}

bool SpatialPartitioningSystems::GetNavMeshWalls(const SpatialPartitioningComponent* spatialPartitioningComponent, const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls)
{
	ARGUS_TRACE(SpatialPartitioningSystems::GetNavMeshWalls);

	ARGUS_RETURN_ON_NULL_BOOL(spatialPartitioningComponent, ArgusECSLog);
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
	queryShapePoints[0].X -= spatialPartitioningComponent->m_validSpaceExtent;
	queryShapePoints[1].X += spatialPartitioningComponent->m_validSpaceExtent;
	queryShapePoints[2].X += spatialPartitioningComponent->m_validSpaceExtent;
	queryShapePoints[3].X -= spatialPartitioningComponent->m_validSpaceExtent;
	queryShapePoints[0].Y += spatialPartitioningComponent->m_validSpaceExtent;
	queryShapePoints[1].Y += spatialPartitioningComponent->m_validSpaceExtent;
	queryShapePoints[2].Y -= spatialPartitioningComponent->m_validSpaceExtent;
	queryShapePoints[3].Y -= spatialPartitioningComponent->m_validSpaceExtent;

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
		FNavigationWallEdge NewEdge;
		for (int32 Idx = 0; Idx < numWalls; Idx++)
		{
			FVector vertex0 = Recast2UnrealPoint(&wallSegments[Idx * 6]);
			FVector vertex1 = Recast2UnrealPoint(&wallSegments[Idx * 6 + 3]);

			bool excluded = ((vertex0.X < -spatialPartitioningComponent->m_validSpaceExtent || vertex0.X > spatialPartitioningComponent->m_validSpaceExtent) ||
							(vertex0.Y < -spatialPartitioningComponent->m_validSpaceExtent || vertex0.Y > spatialPartitioningComponent->m_validSpaceExtent)) &&
							((vertex1.X < -spatialPartitioningComponent->m_validSpaceExtent || vertex1.X > spatialPartitioningComponent->m_validSpaceExtent) ||
							(vertex1.Y < -spatialPartitioningComponent->m_validSpaceExtent || vertex1.Y > spatialPartitioningComponent->m_validSpaceExtent));
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

void SpatialPartitioningSystems::ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, ObstaclesContainer& outObstacles)
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
		for (int32 j = 0; j < outObstacles.Num(); ++j)
		{
			const int32 numObstaclesInChain = outObstacles[j].Num();
			if (numObstaclesInChain == 0)
			{
				continue;
			}

			const FVector2D startOfChainLocation = outObstacles[j].GetHead().m_point;
			const FVector2D endOfChainLocation = outObstacles[j].GetTail().m_point;
			ObstaclePoint pointToAdd;
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
				outObstacles[j].AddObstaclePointsWithFillIn(pointToAdd, true);
				handledEdge = true;
				break;
			}
			if (!matchesStart && matchesEnd)
			{
				outObstacles[j].AddObstaclePointsWithFillIn(pointToAdd, false);
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

		ObstaclePoint vertex0Obstacle, vertex1Obstacle;
		vertex0Obstacle.m_point = edgeVertex0;
		vertex0Obstacle.m_height = edgeVertex0Height;
		vertex1Obstacle.m_point = edgeVertex1;
		vertex1Obstacle.m_height = edgeVertex1Height;
		ObstaclePointArray& array = outObstacles.Emplace_GetRef();
		array.Add(vertex0Obstacle);
		array.AddObstaclePointsWithFillIn(vertex1Obstacle, false);
	}

	const int32 initialSize = outObstacles.Num();
	for (int32 i = (initialSize - 1); i >= 0; --i)
	{
		ObstaclePointArray& appending = outObstacles[i];
		FVector2D appendingObstacleHead = appending.GetHead().m_point;
		FVector2D appendingObstacleTail = appending.GetTail().m_point;

		bool didAppend = false;
		for (int32 j = i - 1; j >= 0; --j)
		{
			ObstaclePointArray& receiving = outObstacles[j];
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
			outObstacles.RemoveAt(i, EAllowShrinking::No);
		}
	}

	outObstacles.Shrink();

	for (int32 i = 0; i < outObstacles.Num(); ++i)
	{
		outObstacles[i].CloseLoop();
		outObstacles[i].Shrink();
		CalculateDirectionAndConvexForObstacles(outObstacles[i]);
	}
}

void SpatialPartitioningSystems::CalculateDirectionAndConvexForObstacles(ObstaclePointArray& outObstacle)
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateDirectionAndConvexForObstacles);

	const int32 numObstaclePoints = outObstacle.Num();
	if (FindAreaOfObstacleCartesian(outObstacle) > 0.0f)
	{
		outObstacle.Reverse();
	}

	float floorHeight = FLT_MAX;
	for (int32 i = 0; i < numObstaclePoints; ++i)
	{
		const int32 nextIndex = (i + 1) % numObstaclePoints;
		outObstacle[i].m_direction = outObstacle[nextIndex].m_point - outObstacle[i].m_point;
		outObstacle[i].m_direction.Normalize();

		const int32 lastIndex = (i - 1) >= 0 ? (i - 1) : numObstaclePoints - 1;
		outObstacle[i].m_isConvex = ArgusMath::IsLeftOfCartesian(outObstacle[lastIndex].m_point, outObstacle[i].m_point, outObstacle[nextIndex].m_point);
	
		floorHeight = outObstacle[i].m_height < floorHeight ? outObstacle[i].m_height : floorHeight;
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);
	outObstacle.m_floorHeight = floorHeight + spatialPartitioningComponent->m_elevatedObstaclePointHeightThreshold;
}

void SpatialPartitioningSystems::DrawDebugObstacles(UWorld* worldPointer, const ObstaclesContainer& obstacles)
{
	if (!worldPointer)
	{
		return;
	}

	if (!CVarShowObstacleDebug.GetValueOnGameThread())
	{
		return;
	}

	for (int32 i = 0; i < obstacles.Num(); ++i)
	{
		for (int32 j = 0; j < obstacles[i].Num(); ++j)
		{
			const float debugHeight = obstacles[i][j].m_height + ArgusECSConstants::k_debugDrawHeightAdjustment;
			DrawDebugString
			(
				worldPointer,
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), debugHeight),
				FString::Printf
				(
					TEXT("%d\nIsConvex: %d\nIsElevated: %d"),
					j,
					obstacles[i][j].m_isConvex,
					obstacles[i].IsPointElevated(j)
				),
				nullptr,
				FColor::Purple,
				60.0f,
				true,
				0.75f
			);
			DrawDebugLine
			(
				worldPointer,
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), debugHeight),
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point + (obstacles[i][j].m_direction * 100.0f)), debugHeight),
				obstacles[i].IsPointElevated(j) ? FColor::Magenta : FColor::Purple,
				true,
				60.0f,
				0u,
				ArgusECSConstants::k_debugDrawLineWidth
			);
			DrawDebugSphere
			(
				worldPointer,
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), debugHeight),
				10.0f,
				4u,
				obstacles[i].IsPointElevated(j) ? FColor::Magenta : FColor::Purple,
				true,
				60.0f,
				0u,
				ArgusECSConstants::k_debugDrawLineWidth
			);
		}
	}
}
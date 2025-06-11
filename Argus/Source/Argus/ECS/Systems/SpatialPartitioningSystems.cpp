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

void SpatialPartitioningSystems::RunSystems(const ArgusEntity& spatialPartitioningEntity, bool didEntityPositionChangeThisFrame)
{
	ARGUS_TRACE(SpatialPartitioningSystems::RunSystems);

	if (!spatialPartitioningEntity)
	{
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = spatialPartitioningEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	if (didEntityPositionChangeThisFrame)
	{
		spatialPartitioningComponent->m_argusEntityKDTree.RebuildKDTreeForAllArgusEntities();
	}

	CacheAdjacentEntityIds(spatialPartitioningComponent);
	CalculateAdjacentEntityGroups();
}

void SpatialPartitioningSystems::CacheAdjacentEntityIds(const SpatialPartitioningComponent* spatialPartitioningComponent)
{
	ARGUS_TRACE(SpatialPartitioningSystems::CacheAdjacentEntityIds);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity entity = ArgusEntity::RetrieveEntity(i);

		if (!entity)
		{
			continue;
		}

		AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>();
		if (!avoidanceGroupingComponent)
		{
			continue;
		}

		avoidanceGroupingComponent->m_adjacentEntities.Reset();
		avoidanceGroupingComponent->m_groupId = ArgusECSConstants::k_maxEntities;
		avoidanceGroupingComponent->m_groupAverageLocation = FVector::ZeroVector;
		avoidanceGroupingComponent->m_numberOfIdleEntities = 0u;

		if (!entity.IsMoveable())
		{
			continue;
		}

		const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
		if (!transformComponent)
		{
			continue;
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
			if (!otherEntity || !otherEntity.IsAlive() || otherEntity.IsPassenger())
			{
				return false;
			}

			return true;
		};

		spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(avoidanceGroupingComponent->m_adjacentEntities, entity, adjacentEntityRange, queryFilter);
	}
}

void SpatialPartitioningSystems::CalculateAdjacentEntityGroups()
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateAdjacentEntityGroups);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		FVector averageLocation = FVector::ZeroVector;
		float numberOfEntitiesInGroup = 0.0f;
		uint16 numberOfStoppedEntities = 0u;
		if (!FloodFillGroupRecursive(i, i, averageLocation, numberOfEntitiesInGroup, numberOfStoppedEntities))
		{
			continue;
		}
		
		ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		if (!entity)
		{
			continue;
		}

		AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>();
		if (!avoidanceGroupingComponent)
		{
			continue;
		}

		avoidanceGroupingComponent->m_groupAverageLocation = ArgusMath::SafeDivide(averageLocation, numberOfEntitiesInGroup);
		avoidanceGroupingComponent->m_numberOfIdleEntities = numberOfStoppedEntities;
	}
}

bool SpatialPartitioningSystems::FloodFillGroupRecursive(uint16 groupId, uint16 entityId, FVector& averageLocation, float& numberOfEntitiesInGroup, uint16& numberOfStoppedEntities)
{
	ArgusEntity entity = ArgusEntity::RetrieveEntity(entityId);
	ArgusEntity groupLeaderEntity = ArgusEntity::RetrieveEntity(groupId);
	if (!entity || !groupLeaderEntity || !entity.IsMoveable())
	{
		return false;
	}

	AvoidanceGroupingComponent* avoidanceGroupingComponent = entity.GetComponent<AvoidanceGroupingComponent>();
	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	const IdentityComponent* identityComponent = entity.GetComponent<IdentityComponent>();
	const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	const IdentityComponent* groupLeaderIdentityComponent = groupLeaderEntity.GetComponent<IdentityComponent>();
	const TargetingComponent* groupLeaderTargetingComponent = groupLeaderEntity.GetComponent<TargetingComponent>();
	if (!avoidanceGroupingComponent || !transformComponent || !identityComponent || !targetingComponent || !groupLeaderIdentityComponent || !groupLeaderTargetingComponent)
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
	if (entity.IsIdle())
	{
		numberOfStoppedEntities++;
	}

	for (int32 i = 0; i < avoidanceGroupingComponent->m_adjacentEntities.Num(); ++i)
	{
		FloodFillGroupRecursive(groupId, avoidanceGroupingComponent->m_adjacentEntities[i], averageLocation, numberOfEntitiesInGroup, numberOfStoppedEntities);
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
	GetNavMeshWalls(navMesh, originLocation, navWalls);

	ConvertWallsIntoObstacles(navWalls, spatialPartitioningComponent->m_obstacles);

	DrawDebugObstacles(worldPointer, spatialPartitioningComponent->m_obstacles);

	spatialPartitioningComponent->m_obstaclePointKDTree.InsertObstaclesIntoKDTree(spatialPartitioningComponent->m_obstacles);
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

bool SpatialPartitioningSystems::GetNavMeshWalls(const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls)
{
	ARGUS_TRACE(SpatialPartitioningSystems::GetNavMeshWalls);

	if (!navMesh)
	{
		return false;
	}

	const FNavigationQueryFilter* filter = navMesh->GetDefaultQueryFilter().Get();
	if (!filter)
	{
		return false;
	}

	const dtNavMesh* detourMesh = navMesh->GetRecastMesh();
	if (!detourMesh)
	{
		return false;
	}

	const uint32 maxSearchNodes = filter->GetMaxSearchNodes();
	const FRecastQueryFilter* recastQueryFilter = static_cast<const FRecastQueryFilter*>(filter->GetImplementation());
	if (!recastQueryFilter)
	{
		return false;
	}

	const dtQueryFilter* queryFilter = recastQueryFilter->GetAsDetourQueryFilter();
	if (!queryFilter)
	{
		return false;
	}

	int32 numWalls = 0;
	FVector::FReal wallSegments[ArgusECSConstants::k_maxDetourWalls * 3 * 2] = { 0 };
	dtPolyRef wallPolys[ArgusECSConstants::k_maxDetourWalls * 2] = { 0 };

	int32 numNeis = 0;
	dtPolyRef neiPolys[ArgusECSConstants::k_maxDetourPolys] = { 0 };

	const int verts = 4;
	TArray<FVector> queryShapePoints;
	queryShapePoints.SetNumZeroed(verts);
	queryShapePoints[0].X -= ArgusECSConstants::k_detourQuerySize;
	queryShapePoints[1].X += ArgusECSConstants::k_detourQuerySize;
	queryShapePoints[2].X += ArgusECSConstants::k_detourQuerySize;
	queryShapePoints[3].X -= ArgusECSConstants::k_detourQuerySize;
	queryShapePoints[0].Y += ArgusECSConstants::k_detourQuerySize;
	queryShapePoints[1].Y += ArgusECSConstants::k_detourQuerySize;
	queryShapePoints[2].Y -= ArgusECSConstants::k_detourQuerySize;
	queryShapePoints[3].Y -= ArgusECSConstants::k_detourQuerySize;

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

			bool excluded = ((vertex0.X < -ArgusECSConstants::k_detourQuerySize || vertex0.X > ArgusECSConstants::k_detourQuerySize) || 
							(vertex0.Y < -ArgusECSConstants::k_detourQuerySize || vertex0.Y > ArgusECSConstants::k_detourQuerySize)) &&
							((vertex1.X < -ArgusECSConstants::k_detourQuerySize || vertex1.X > ArgusECSConstants::k_detourQuerySize) || 
							(vertex1.Y < -ArgusECSConstants::k_detourQuerySize || vertex1.Y > ArgusECSConstants::k_detourQuerySize));
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

void SpatialPartitioningSystems::ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, TArray<ObstaclePointArray>& outObstacles)
{
	ARGUS_TRACE(SpatialPartitioningSystems::ConvertWallsIntoObstacles);

	const int32 numNavEdges = navEdges.Num();
	if ((numNavEdges % 2) != 0 || numNavEdges == 0)
	{
		return;
	}

	for (int32 i = 0; i < numNavEdges; i += 2)
	{
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
			}
			if (startOfChainLocation == edgeVertex1)
			{
				matchesStart = true;
				pointToAdd.m_point = edgeVertex0;
			}
			if (endOfChainLocation == edgeVertex0)
			{
				matchesEnd = true;
				pointToAdd.m_point = edgeVertex1;
			}
			if (endOfChainLocation == edgeVertex1)
			{
				matchesEnd = true;
				pointToAdd.m_point = edgeVertex0;
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
		vertex1Obstacle.m_point = edgeVertex1;
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

	for (int32 i = 0; i < numObstaclePoints; ++i)
	{
		const int32 nextIndex = (i + 1) % numObstaclePoints;
		outObstacle[i].m_direction = outObstacle[nextIndex].m_point - outObstacle[i].m_point;
		outObstacle[i].m_direction.Normalize();

		const int32 lastIndex = (i - 1) >= 0 ? (i - 1) : numObstaclePoints - 1;
		outObstacle[i].m_isConvex = ArgusMath::IsLeftOfCartesian(outObstacle[lastIndex].m_point, outObstacle[i].m_point, outObstacle[nextIndex].m_point);
	}
}

void SpatialPartitioningSystems::DrawDebugObstacles(UWorld* worldPointer, const TArray<ObstaclePointArray>& obstacles)
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
			DrawDebugString
			(
				worldPointer,
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), ArgusECSConstants::k_debugDrawHeightAdjustment),
				FString::Printf
				(
					TEXT("%d\nIsConvex: %d"),
					j,
					obstacles[i][j].m_isConvex
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
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), ArgusECSConstants::k_debugDrawHeightAdjustment),
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point + (obstacles[i][j].m_direction * 100.0f)), ArgusECSConstants::k_debugDrawHeightAdjustment),
				FColor::Purple,
				true,
				60.0f,
				0u,
				ArgusECSConstants::k_debugDrawLineWidth
			);
			DrawDebugSphere
			(
				worldPointer,
				FVector(ArgusMath::ToUnrealVector2(obstacles[i][j].m_point), ArgusECSConstants::k_debugDrawHeightAdjustment),
				10.0f,
				4u,
				FColor::Purple,
				true,
				60.0f,
				0u,
				ArgusECSConstants::k_debugDrawLineWidth
			);
		}
	}
}
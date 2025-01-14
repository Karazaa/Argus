// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpatialPartitioningSystems.h"
#include "ArgusDetourQuery.h"
#include "ArgusECSConstants.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastHelpers.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/RecastQueryFilter.h"

static TAutoConsoleVariable<bool> CVarShowObstacleDebug(TEXT("Argus.SpatialPartitioning.ShowAvoidanceObstacleDebug"), false, TEXT(""));

void SpatialPartitioningSystems::RunSystems(const ArgusEntity& spatialPartitioningEntity)
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

	spatialPartitioningComponent->m_argusEntityKDTree.ResetKDTreeWithAverageLocation();

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

		SpatialPartitioningSystemsComponentArgs components;
		components.m_entity = retrievedEntity;
		components.m_transformComponent = transformComponent;
		components.m_singletonSpatialParitioningComponent = spatialPartitioningComponent;

		spatialPartitioningComponent->m_argusEntityKDTree.InsertArgusEntityIntoKDTree(retrievedEntity);
	}
}

bool SpatialPartitioningSystems::SpatialPartitioningSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (!m_entity || !m_singletonSpatialParitioningComponent)
	{
		ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(SpatialPartitioningSystemsComponentArgs));
		return false;
	}
	return true;
}

void SpatialPartitioningSystems::CalculateAvoidanceObstacles(UWorld* worldPointer)
{
	ARGUS_TRACE(SpatialPartitioningSystems::CalculateAvoidanceObstacles);

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

	TArray<FVector> navWalls;
	GetNavMeshWalls(navMesh, originLocation, navWalls);

	TArray<TArray<ObstaclePoint>> obstacles;
	ConvertWallsIntoObstacles(navWalls, obstacles);

	DebugDrawObstacles(worldPointer, obstacles);
}

float SpatialPartitioningSystems::FindAreaOfObstacleCartesian(const TArray<ObstaclePoint>& obstaclePoints)
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

void SpatialPartitioningSystems::ConvertWallsIntoObstacles(const TArray<FVector>& navEdges, TArray<TArray<ObstaclePoint>>& outObstacles)
{
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

			const FVector2D startOfChainLocation = outObstacles[j][0].m_point;
			const FVector2D endOfChainLocation = outObstacles[j][numObstaclesInChain - 1].m_point;
			if (FVector2D::DistSquared(startOfChainLocation, edgeVertex0) < 5.0f)
			{
				ObstaclePoint vertex1Obstacle;
				vertex1Obstacle.m_point = edgeVertex1;
				outObstacles[j].Insert(vertex1Obstacle, 0);
				handledEdge = true;
				break;
			}
			if (FVector2D::DistSquared(startOfChainLocation, edgeVertex1) < 5.0f)
			{
				ObstaclePoint vertex0Obstacle;
				vertex0Obstacle.m_point = edgeVertex0;
				outObstacles[j].Insert(vertex0Obstacle, 0);
				handledEdge = true;
				break;
			}
			if (FVector2D::DistSquared(endOfChainLocation, edgeVertex0) < 5.0f)
			{
				ObstaclePoint vertex1Obstacle;
				vertex1Obstacle.m_point = edgeVertex1;
				outObstacles[j].Add(vertex1Obstacle);
				handledEdge = true;
				break;
			}
			if (FVector2D::DistSquared(endOfChainLocation, edgeVertex1) < 5.0f)
			{
				ObstaclePoint vertex0Obstacle;
				vertex0Obstacle.m_point = edgeVertex0;
				outObstacles[j].Add(vertex0Obstacle);
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
		outObstacles.Add(TArray<ObstaclePoint>({ vertex0Obstacle, vertex1Obstacle }));
	}

	for (int32 i = 0; i < outObstacles.Num(); ++i)
	{
		CalculateDirectionAndConvexForObstacles(outObstacles[i]);
	}
}

void SpatialPartitioningSystems::CalculateDirectionAndConvexForObstacles(TArray<ObstaclePoint>& outObstacle)
{
	const int32 numObstaclePoints = outObstacle.Num();
	if (FindAreaOfObstacleCartesian(outObstacle) > 0.0f)
	{
		const int32 halfObstaclePoints = numObstaclePoints / 2;
		for (int32 i = 0; i < halfObstaclePoints; ++i)
		{
			outObstacle.Swap(i, numObstaclePoints - (i + 1));
		}
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

void SpatialPartitioningSystems::DebugDrawObstacles(UWorld* worldPointer, const TArray<TArray<ObstaclePoint>>& obstacles)
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
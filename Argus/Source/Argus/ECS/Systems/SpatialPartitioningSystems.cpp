// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpatialPartitioningSystems.h"
#include "ArgusDetourQuery.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastHelpers.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/RecastQueryFilter.h"

void SpatialPartitioningSystems::RunSystems(const ArgusEntity& spatialPartitioningEntity)
{
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

	FVector heightAdjustment = FVector(0.0f, 0.0f, 5.0f);
	for (int32 i = 0; i < navWalls.Num(); i += 2)
	{
		DrawDebugLine(worldPointer, navWalls[i] + heightAdjustment, navWalls[i + 1] + heightAdjustment, FColor::Red, true, 1.0f, 0u, 5.0f);
	}
}

bool SpatialPartitioningSystems::GetNavMeshWalls(const ARecastNavMesh* navMesh, const FNavLocation& originLocation, TArray<FVector>& outNavWalls)
{
	if (!navMesh)
	{
		return false;
	}

	const FNavigationQueryFilter* filter = navMesh->GetDefaultQueryFilter().Get();
	if (!filter)
	{
		return false;
	}

	dtNavMeshQuery query;
	const dtNavMesh* detourMesh = navMesh->GetRecastMesh();
	if (!detourMesh)
	{
		return false;
	}

	const uint32 maxSearchNodes = filter->GetMaxSearchNodes();
	query.init(detourMesh, maxSearchNodes);
	
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

	const int32 maxWalls = 1028;
	int32 numWalls = 0;
	FVector::FReal wallSegments[maxWalls * 3 * 2] = { 0 };
	dtPolyRef wallPolys[maxWalls * 2] = { 0 };

	const int32 maxNeis = 1028;
	int32 numNeis = 0;
	dtPolyRef neiPolys[maxNeis] = { 0 };

	const float querySize = 10000.0f;
	const int verts = 4;
	TArray<FVector> queryShapePoints;
	queryShapePoints.SetNumZeroed(verts);
	queryShapePoints[0].X -= querySize;
	queryShapePoints[1].X += querySize;
	queryShapePoints[2].X += querySize;
	queryShapePoints[3].X -= querySize;
	queryShapePoints[0].Y += querySize;
	queryShapePoints[1].Y += querySize;
	queryShapePoints[2].Y -= querySize;
	queryShapePoints[3].Y -= querySize;

	FVector::FReal rcConvexPolygon[verts * 3] = { 0 };

	for (int32 i = 0; i < verts; i++)
	{
		const FVector RcPoint = Unreal2RecastPoint(queryShapePoints[i]);
		rcConvexPolygon[i * 3 + 0] = RcPoint.X;
		rcConvexPolygon[i * 3 + 1] = RcPoint.Y;
		rcConvexPolygon[i * 3 + 2] = RcPoint.Z;
	}

	dtStatus queryStatus = ArgusDetourQuery::FindWallsOverlappingShape(originLocation.NodeRef, rcConvexPolygon, verts, queryFilter, neiPolys, &numNeis, maxNeis, wallSegments, wallPolys, &numWalls, maxWalls);

	if (dtStatusSucceed(queryStatus))
	{
		outNavWalls.Reset(numWalls * 2);
		FNavigationWallEdge NewEdge;
		for (int32 Idx = 0; Idx < numWalls; Idx++)
		{
			outNavWalls.Add(Recast2UnrealPoint(&wallSegments[Idx * 6]));
			outNavWalls.Add(Recast2UnrealPoint(&wallSegments[Idx * 6 + 3]));
		}

		return true;
	}

	return false;
}
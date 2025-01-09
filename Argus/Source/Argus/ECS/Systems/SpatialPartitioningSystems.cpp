// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpatialPartitioningSystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Detour/DetourNavMeshQuery.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"

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

void SpatialPartitioningSystems::CalculateAvoidanceObstacles(UWorld* worldPointer)
{
	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in %s is nullptr."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer);
	if (!unrealNavigationSystem)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UNavigationSystemV1*));
		return;
	}

	ARecastNavMesh* navMesh = Cast<ARecastNavMesh>(unrealNavigationSystem->MainNavData);
	if (!navMesh)
	{
		// ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ANavigationData*));
		return;
	}

	FNavLocation originLocation;
	if (!unrealNavigationSystem->ProjectPointToNavigation(FVector::ZeroVector, originLocation))
	{
		return;
	}

	dtNavMeshQuery query;
	const uint32 maxSearchNodes = navMesh->GetDefaultQueryFilter().Get()->GetMaxSearchNodes();
	query.init(navMesh->GetRecastMesh(), maxSearchNodes);
	//dtReal center[3];
	//dtReal radius;
	//dtStatus Status = query.findWallsInNeighbourhood(originLocation.NodeRef, center, radius, );

	//TArray<FVector> navEdges;
	//for (int32 i = 0; i < navEdges.Num(); i += 2)
	//{
	//	DrawDebugLine(worldPointer, navEdges[i], navEdges[i + 1], FColor::Red, false, 10.0f, 0u, 5.0f);
	//}
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
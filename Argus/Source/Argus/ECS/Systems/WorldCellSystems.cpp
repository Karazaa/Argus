// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "WorldCellSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "SystemArgumentDefinitions/WorldCellSystemsArgs.h"

void WorldCellSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(WorldCellSystems::RunSystems);

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	ArgusIterators::IterateSystemsArgs<WorldCellSystemsArgs>([spatialPartitioningComponent](const WorldCellSystemsArgs& components)
	{
		UpdateWorldCellLocationPerEntity(components, spatialPartitioningComponent);
	});
}

void WorldCellSystems::UpdateWorldCellLocationPerEntity(const WorldCellSystemsArgs& components, const SpatialPartitioningComponent* spatialPartitioningComponent)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const float cellX = ArgusMath::SafeDivide(components.m_transformComponent->m_location.Y, spatialPartitioningComponent->m_worldCellExtent);
	const float cellY = ArgusMath::SafeDivide(components.m_transformComponent->m_location.X, spatialPartitioningComponent->m_worldCellExtent);
	
	components.m_worldCellComponent->m_cellXCoordinate = cellX < 0.0f ? FMath::CeilToInt32(cellX) : FMath::FloorToInt32(cellX);
	components.m_worldCellComponent->m_cellYCoordinate = cellY < 0.0f ? FMath::CeilToInt32(cellY) : FMath::FloorToInt32(cellY);
}

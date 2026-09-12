// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DynamicAllocComponentDefinitions/SpatialPartitioningComponent.h"
#include "ArgusStaticData.h"
#include "RecordDefinitions/ObstaclesRecord.h"
#include "RecordDefinitions/WorldCellRecord.h"

namespace
{
	FObstaclesContainer k_invalidObstaclesContainer;
	FObstaclePoint k_invalidObstaclePoint;
}

const FObstaclesContainer& SpatialPartitioningComponent::GetObstalcesContainer() const
{
	const UWorldCellRecord* worldCellRecord = ArgusStaticData::GetRecord<UWorldCellRecord>(m_initialWorldCell.GetId());
	ARGUS_RETURN_ON_NULL_VALUE(worldCellRecord, ArgusECSLog, k_invalidObstaclesContainer);
	
	const UObstaclesRecord* obstaclesRecord = ArgusStaticData::GetRecord<UObstaclesRecord>(worldCellRecord->m_obstaclesRecord.GetId());
	ARGUS_RETURN_ON_NULL_VALUE(obstaclesRecord, ArgusECSLog, k_invalidObstaclesContainer);

	return obstaclesRecord->m_obstaclesContainer;
}

const FObstaclePoint& SpatialPartitioningComponent::GetObstaclePointFromIndicies(const ObstacleIndicies& indicies) const
{
	return GetObstalcesContainer().m_obstacleArrays[indicies.m_obstacleIndex].m_obstaclePoints[indicies.m_obstaclePointIndex];
}

const FObstaclePoint& SpatialPartitioningComponent::GetNextObstaclePointFromIndicies(const ObstacleIndicies& indicies) const
{
	return GetObstalcesContainer().m_obstacleArrays[indicies.m_obstacleIndex].GetNext(indicies.m_obstaclePointIndex);
}

bool SpatialPartitioningComponent::IsPointElevated(const ObstacleIndicies& indicies) const
{
	return GetObstalcesContainer().m_obstacleArrays[indicies.m_obstacleIndex].IsPointElevated(indicies.m_obstaclePointIndex);
}

bool SpatialPartitioningComponent::IsNextPointElevated(const ObstacleIndicies& indicies) const
{
	return GetObstalcesContainer().m_obstacleArrays[indicies.m_obstacleIndex].IsNextPointElevated(indicies.m_obstaclePointIndex);
}
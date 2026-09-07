// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"
#include "ComponentDependencies/ObstaclePointKDTree.h"
#include "CoreMinimal.h"

struct SpatialPartitioningComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ArgusEntityKDTree m_argusEntityKDTree;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ArgusEntityKDTree m_flyingArgusEntityKDTree;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ObstaclePointKDTree m_obstaclePointKDTree;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	FObstaclesContainer m_obstacles;

	float m_validSpaceExtent = 3000.0f;
	float m_flyingPlaneHeight = 300.0f;
	float m_elevatedObstaclePointHeightThreshold = 10.0f;

	const FObstaclePoint& GetObstaclePointFromIndicies(const ObstacleIndicies& indicies) const 
	{
		return m_obstacles.m_obstacleArrays[indicies.m_obstacleIndex].m_obstaclePoints[indicies.m_obstaclePointIndex];
	}

	const FObstaclePoint& GetNextObstaclePointFromIndicies(const ObstacleIndicies& indicies) const
	{
		return m_obstacles.m_obstacleArrays[indicies.m_obstacleIndex].GetNext(indicies.m_obstaclePointIndex);
	}

	bool IsPointElevated(const ObstacleIndicies& indicies) const
	{
		return m_obstacles.m_obstacleArrays[indicies.m_obstacleIndex].IsPointElevated(indicies.m_obstaclePointIndex);
	}

	bool IsNextPointElevated(const ObstacleIndicies& indicies) const
	{
		return m_obstacles.m_obstacleArrays[indicies.m_obstacleIndex].IsNextPointElevated(indicies.m_obstaclePointIndex);
	}
};
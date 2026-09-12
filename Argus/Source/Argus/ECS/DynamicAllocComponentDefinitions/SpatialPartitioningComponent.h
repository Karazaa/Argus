// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"
#include "ComponentDependencies/ObstaclePointKDTree.h"
#include "RecordReferences/WorldCellRecordReference.h"

struct SpatialPartitioningComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ArgusEntityKDTree m_argusEntityKDTree;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ArgusEntityKDTree m_flyingArgusEntityKDTree;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ObstaclePointKDTree m_obstaclePointKDTree;

	FUWorldCellRecordReference m_initialWorldCell;

	float m_validSpaceExtent = 3000.0f;
	float m_flyingPlaneHeight = 300.0f;
	float m_elevatedObstaclePointHeightThreshold = 10.0f;

	const FObstaclesContainer& GetObstalcesContainer() const;
	const FObstaclePoint& GetObstaclePointFromIndicies(const ObstacleIndicies& indicies) const;
	const FObstaclePoint& GetNextObstaclePointFromIndicies(const ObstacleIndicies& indicies) const;
	bool IsPointElevated(const ObstacleIndicies& indicies) const;
	bool IsNextPointElevated(const ObstacleIndicies& indicies) const;
};
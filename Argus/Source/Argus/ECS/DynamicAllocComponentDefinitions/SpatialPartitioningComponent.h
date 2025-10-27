// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"
#include "ComponentDependencies/ObstaclePointKDTree.h"
#include "CoreMinimal.h"

struct SpatialPartitioningComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_IGNORE()
	ArgusEntityKDTree m_argusEntityKDTree;

	ARGUS_IGNORE()
	ArgusEntityKDTree m_flyingArgusEntityKDTree;

	ARGUS_IGNORE()
	ObstaclePointKDTree m_obstaclePointKDTree;

	ARGUS_IGNORE()
	TArray<ObstaclePointArray> m_obstacles;

	float m_validSpaceExtent = 3000.0f;
	float m_flyingPlaneHeight = 300.0f;
};
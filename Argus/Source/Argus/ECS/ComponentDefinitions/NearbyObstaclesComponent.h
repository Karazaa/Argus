// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ObstaclePointKDTree.h"
#include "CoreMinimal.h"

struct NearbyObstaclesComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	ObstaclePointKDTreeRangeOutput m_obstacleIndicies;
};

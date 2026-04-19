// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ObstaclePointKDTree.h"
#include "CoreMinimal.h"

struct NearbyObstaclesComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ObstaclePointKDTreeRangeOutput m_obstacleIndicies;
};

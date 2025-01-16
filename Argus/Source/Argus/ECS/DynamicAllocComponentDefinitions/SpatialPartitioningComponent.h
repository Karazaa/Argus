// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"
#include "ComponentDependencies/ObstaclePointKDTree.h"
#include "CoreMinimal.h"

struct SpatialPartitioningComponent
{
	ARGUS_IGNORE()
	ArgusEntityKDTree m_argusEntityKDTree;

	ARGUS_IGNORE()
	ObstaclePointKDTree m_obstaclePointKDTree;

	ARGUS_IGNORE()
	TArray<ObstaclePointArray> m_obstacles;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append(FString::Printf(TEXT("\n[%s]"), ARGUS_NAMEOF(SpatialPartitioningComponent)));
	}
};
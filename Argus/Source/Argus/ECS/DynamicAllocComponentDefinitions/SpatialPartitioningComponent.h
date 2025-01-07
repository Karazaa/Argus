// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"
#include "CoreMinimal.h"

struct SpatialPartitioningComponent
{
	ARGUS_IGNORE()
	ArgusEntityKDTree m_argusEntityKDTree;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append(FString::Printf(TEXT("\n[%s]"), ARGUS_NAMEOF(SpatialPartitioningComponent)));
	}
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusKDTree.h"
#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct SpatialPartitioningComponent
{
	ARGUS_IGNORE()
	ArgusKDTree m_argusKDTree;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append(FString::Printf(TEXT("\n[%s]"), ARGUS_NAMEOF(SpatialPartitioningComponent)));
	}
};
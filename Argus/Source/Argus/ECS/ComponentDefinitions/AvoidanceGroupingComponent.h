// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusMacros.h"

struct AvoidanceGroupingComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_IGNORE()
	TArray<uint16> m_adjacentEntities;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s]"),
				ARGUS_NAMEOF(AvoidanceGroupingComponent)
			)
		);
	}
};
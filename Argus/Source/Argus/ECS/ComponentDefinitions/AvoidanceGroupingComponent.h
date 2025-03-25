// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusECSConstants.h"
#include "ArgusMacros.h"

struct AvoidanceGroupingComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_IGNORE()
	TArray<uint16> m_adjacentEntities;

	ARGUS_IGNORE()
	FVector m_groupAverageLocation = FVector::ZeroVector;

	ARGUS_IGNORE()
	uint16 m_groupId = ArgusECSConstants::k_maxEntities;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s]\n    (%s, %d)"),
				ARGUS_NAMEOF(AvoidanceGroupingComponent),
				ARGUS_NAMEOF(m_groupId),
				m_groupId
			)
		);
	}
};
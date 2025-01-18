// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusQueue.h"

struct NavigationComponent
{
	ARGUS_IGNORE()
	TArray<FVector> m_navigationPoints;

	ARGUS_IGNORE()
	ArgusQueue<FVector> m_queuedWaypoints;

	ARGUS_IGNORE()
	int32 m_lastPointIndex = 0;

	void ResetPath()
	{
		m_navigationPoints.Empty();
		m_lastPointIndex = 0u;
	}

	void ResetQueuedWaypoints()
	{
		m_queuedWaypoints.Empty();
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %d) \n    (Num points: %d) \n    (Num queued waypoints: %s)"), 
				ARGUS_NAMEOF(NavigationComponent),
				ARGUS_NAMEOF(m_lastPointIndex),
				m_lastPointIndex,
				m_navigationPoints.Num(),
				m_queuedWaypoints.IsEmpty() ? TEXT("No") : TEXT("Yes")
			)
		);
	}
};
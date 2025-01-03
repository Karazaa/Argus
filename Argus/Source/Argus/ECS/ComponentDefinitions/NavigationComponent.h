// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include <queue>
#include <vector>

struct NavigationComponent
{
	ARGUS_IGNORE()
	std::vector<FVector> m_navigationPoints = std::vector<FVector>();

	ARGUS_IGNORE()
	std::queue<FVector> m_queuedWaypoints = std::queue<FVector>();

	ARGUS_IGNORE()
	uint16 m_lastPointIndex = 0u;

	void ResetPath()
	{
		m_navigationPoints.clear();
		m_lastPointIndex = 0u;
	}

	void ResetQueuedWaypoints()
	{
		while (m_queuedWaypoints.size() > 0)
		{
			m_queuedWaypoints.pop();
		}
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %d) \n    (Num points: %d) \n    (Num queued waypoints: %d)"), 
				ARGUS_NAMEOF(NavigationComponent),
				ARGUS_NAMEOF(m_lastPointIndex),
				m_lastPointIndex,
				m_navigationPoints.size(),
				m_queuedWaypoints.size()
			)
		);
	}
};
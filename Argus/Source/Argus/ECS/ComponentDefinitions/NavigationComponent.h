// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusDeque.h"
#include "ComponentDependencies/NavigationWaypoint.h"

struct NavigationComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_NO_DATA
	TArray<FVector, ArgusContainerAllocator<15u> > m_navigationPoints;

	ARGUS_NO_DATA
	ArgusDeque<NavigationWaypoint, ArgusContainerAllocator<5u> > m_queuedWaypoints;

	ARGUS_NO_DATA
	int32 m_lastPointIndex = 0;

	void ResetPath()
	{
		m_navigationPoints.Reset();
		m_lastPointIndex = 0u;
	}

	void ResetQueuedWaypoints()
	{
		m_queuedWaypoints.Reset();
	}

	bool HasValidNextIndex() const 
	{
		return m_navigationPoints.IsValidIndex(m_lastPointIndex + 1);
	}
};
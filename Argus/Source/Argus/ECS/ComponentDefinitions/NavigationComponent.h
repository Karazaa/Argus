// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "AI/Navigation/NavAgentSelector.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusDeque.h"
#include "ComponentDependencies/NavigationWaypoint.h"

struct NavigationComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_COMP_NO_DATA
	TArray<FVector, ArgusContainerAllocator<15u> > m_navigationPoints;

	ARGUS_COMP_NO_DATA
	ArgusDeque<NavigationWaypoint, ArgusContainerAllocator<5u> > m_queuedWaypoints;

	ARGUS_COMP_NO_DATA
	int32 m_lastPointIndex = 0;

	ARGUS_COMP_NO_DATA
	int32 m_groupLastPointIndex = 0;

	ARGUS_COMP_NO_DATA
	FNavAgentSelector m_currentNavAgentToUse = FNavAgentSelector(0u);

	FNavAgentSelector m_navAgentToUseWhenSolo = FNavAgentSelector(1u);

	void ResetPath()
	{
		m_navigationPoints.Reset();
		m_lastPointIndex = 0;
		m_groupLastPointIndex = 0;
	}

	void ResetQueuedWaypoints()
	{
		m_queuedWaypoints.Reset();
	}

	bool HasValidNextIndex() const 
	{
		return m_navigationPoints.IsValidIndex(m_lastPointIndex + 1);
	}

	bool HasValidNextGroupIndex() const
	{
		return m_navigationPoints.IsValidIndex(m_groupLastPointIndex + 1);
	}
};
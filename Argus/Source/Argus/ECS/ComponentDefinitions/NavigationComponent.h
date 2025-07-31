// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusDeque.h"

struct NavigationComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	TArray<FVector, ArgusContainerAllocator<15> > m_navigationPoints;

	ARGUS_IGNORE()
	ArgusDeque<FVector, 5> m_queuedWaypoints;

	ARGUS_IGNORE()
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
};
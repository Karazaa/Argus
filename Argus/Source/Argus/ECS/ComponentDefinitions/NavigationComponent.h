// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusDeque.h"

struct NavigationComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	TArray<FVector> m_navigationPoints;

	ARGUS_IGNORE()
	ArgusDeque<FVector> m_queuedWaypoints;

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
};
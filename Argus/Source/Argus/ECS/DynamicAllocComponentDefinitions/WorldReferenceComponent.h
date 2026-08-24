// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/Teams.h"

class UWorld;

struct WorldReferenceComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	UWorld* m_worldPointer = nullptr;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint64 m_frameCounter = 0u;

	bool IncrementFrameCounter()
	{
		if (m_frameCounter == UINT64_MAX)
		{
			m_frameCounter = 0;
			return true;
		}

		m_frameCounter++;
		return false;
	}

	ETeam GetFrameDeferredRelevantTeam() const
	{
		const uint64 index = m_frameCounter % static_cast<uint64>(NUM_TEAMS);
		return static_cast<ETeam>(1 << static_cast<uint8>(index));
	}
};

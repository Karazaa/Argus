// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/TeamCommanderPriorities.h"
#include "ComponentDependencies/Teams.h"
#include "Containers/BitArray.h"

struct TeamCommanderComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_idleEntityIdsForTeam;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_seenResourceSourceEntityIds;

	ARGUS_IGNORE()
	TArray<TeamCommanderPriority, ArgusContainerAllocator<2u> > m_priorities;

	ARGUS_IGNORE()
	TBitArray<ArgusContainerAllocator<0u> > m_revealedAreas;

	float m_revealedAreaDimensionLength = 800.0f;

	ARGUS_IGNORE()
	uint16 m_numResourceExtractors = 0u;

	ARGUS_IGNORE()
	uint16 m_numResourceSinks = 0u;

	ARGUS_IGNORE()
	uint16 m_numLivingUnits = 0u;

	ARGUS_IGNORE()
	ETeam m_teamToCommand = ETeam::None;
};

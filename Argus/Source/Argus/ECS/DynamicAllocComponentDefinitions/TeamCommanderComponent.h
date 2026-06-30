// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"
#include "ArgusMap.h"
#include "ArgusSetAllocator.h"
#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDependencies/TeamCommanderPriorities.h"
#include "ComponentDependencies/Teams.h"
#include "Containers/BitArray.h"

struct TeamCommanderComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	TArray<uint16, ArgusContainerAllocator<10u> > m_idleEntityIdsForTeam;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ArgusMap<uint16, ConstructionData, ArgusSetAllocator<10> > m_inProgressConstructionData;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	TArray<uint32, ArgusContainerAllocator<10u>> m_spawningEntityRecordIds; 

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	TArray<TeamCommanderPriority, ArgusContainerAllocator<11u> > m_priorities;

	ARGUS_COMP_NO_DATA
	TBitArray<ArgusContainerAllocator<0u> > m_revealedAreas;

	float m_revealedAreaDimensionLength = 800.0f;

	ARGUS_COMP_NO_DATA
	ETeam m_teamToCommand = ETeam::None;

	void ResetUpdateArrays()
	{
		m_idleEntityIdsForTeam.Reset();
		m_inProgressConstructionData.Reset();
		m_spawningEntityRecordIds.Reset();
		m_priorities.Reset();
	}

	template <typename Function>
	void IterateRevealedAreas(bool searchForRevealed, Function&& function) const
	{
		int32 currentIndex = 0u;
		while (currentIndex < m_revealedAreas.Num() && currentIndex >= 0)
		{
			if (m_revealedAreas[currentIndex] == searchForRevealed)
			{
				function(currentIndex);
			}

			currentIndex = m_revealedAreas.FindFrom(searchForRevealed, currentIndex + 1);
		}
	}
};

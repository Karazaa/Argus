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
#include "Containers/Set.h"

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

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	TSet<uint32> m_availableAbilityRecordIds;

	ARGUS_COMP_NO_DATA
	TBitArray<ArgusContainerAllocator<0u> > m_revealedAreas;

	float m_revealedAreaDimensionLength = 800.0f;

	ARGUS_COMP_NO_DATA
	ETeam m_teamToCommand = ETeam::None;

	ARGUS_COMP_NO_DATA
	BITMASK_ETeam m_allies = 0u;

	ARGUS_COMP_NO_DATA
	BITMASK_ETeam m_enemies = 0u;

	void ResetUpdateArrays()
	{
		m_idleEntityIdsForTeam.Reset();
		m_inProgressConstructionData.Reset();
		m_spawningEntityRecordIds.Reset();
		m_priorities.Reset();
		m_availableAbilityRecordIds.Reset();
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

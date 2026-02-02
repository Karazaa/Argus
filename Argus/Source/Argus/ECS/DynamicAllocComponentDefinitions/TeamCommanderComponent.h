// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDependencies/TeamCommanderPriorities.h"
#include "ComponentDependencies/Teams.h"
#include "Containers/BitArray.h"

struct TeamCommanderComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_idleEntityIdsForTeam;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_seenResourceASourceEntityIds;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_seenResourceBSourceEntityIds;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_seenResourceCSourceEntityIds;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_resourceASinkEntityIds;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_resourceBSinkEntityIds;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<10u> > m_resourceCSinkEntityIds;

	ARGUS_IGNORE()
	TArray<TeamCommanderPriority, ArgusContainerAllocator<7u> > m_priorities;

	ARGUS_IGNORE()
	TBitArray<ArgusContainerAllocator<0u> > m_revealedAreas;

	float m_revealedAreaDimensionLength = 800.0f;

	ARGUS_IGNORE()
	ETeam m_teamToCommand = ETeam::None;

	void ResetUpdateArrays()
	{
		m_idleEntityIdsForTeam.Reset();
		m_seenResourceASourceEntityIds.Reset();
		m_seenResourceBSourceEntityIds.Reset();
		m_seenResourceCSourceEntityIds.Reset();
		m_resourceASinkEntityIds.Reset();
		m_resourceBSinkEntityIds.Reset();
		m_resourceCSinkEntityIds.Reset();
		m_priorities.Reset();
	}

	TArray<uint16, ArgusContainerAllocator<10u> >& GetSeenSourceEntityIdsForResourceType(EResourceType type)
	{
		switch (type)
		{
			case EResourceType::ResourceA:
				return m_seenResourceASourceEntityIds;
			case EResourceType::ResourceB:
				return m_seenResourceBSourceEntityIds;
			case EResourceType::ResourceC:
				return m_seenResourceCSourceEntityIds;
			default:
				break;
		}

		return m_seenResourceASourceEntityIds;
	}

	TArray<uint16, ArgusContainerAllocator<10u> >& GetSinkEntityIdsForResourceType(EResourceType type)
	{
		switch (type)
		{
			case EResourceType::ResourceA:
				return m_resourceASinkEntityIds;
			case EResourceType::ResourceB:
				return m_resourceBSinkEntityIds;
			case EResourceType::ResourceC:
				return m_resourceCSinkEntityIds;
			default:
				break;
		}

		return m_resourceASinkEntityIds;
	}

	template <typename Function>
	void IterateSeenResourceSourcesOfType(EResourceType type, Function&& function)
	{
		TArray<uint16, ArgusContainerAllocator<10u> >& sources = GetSeenSourceEntityIdsForResourceType(type);
		for (int32 i = 0; i < sources.Num(); ++i)
		{
			function(sources[i]);
		}
	}

	template <typename Function>
	void IterateResourceSinksOfType(EResourceType type, Function&& function)
	{
		TArray<uint16, ArgusContainerAllocator<10u> >& sinks = GetSinkEntityIdsForResourceType(type);
		for (int32 i = 0; i < sinks.Num(); ++i)
		{
			function(sinks[i]);
		}
	}

	template <typename Function>
	void IterateAllSeenResourceSources(Function&& function) const 
	{
		for (int32 i = 0; i < m_seenResourceASourceEntityIds.Num(); ++i)
		{
			function(m_seenResourceASourceEntityIds[i]);
		}
		for (int32 i = 0; i < m_seenResourceBSourceEntityIds.Num(); ++i)
		{
			function(m_seenResourceBSourceEntityIds[i]);
		}
		for (int32 i = 0; i < m_seenResourceCSourceEntityIds.Num(); ++i)
		{
			function(m_seenResourceCSourceEntityIds[i]);
		}
	}

	template <typename Function>
	void IterateAllResourceSinks(Function&& function) const 
	{
		for (int32 i = 0; i < m_resourceASinkEntityIds.Num(); ++i)
		{
			function(m_resourceASinkEntityIds[i]);
		}
		for (int32 i = 0; i < m_resourceBSinkEntityIds.Num(); ++i)
		{
			function(m_resourceBSinkEntityIds[i]);
		}
		for (int32 i = 0; i < m_resourceCSinkEntityIds.Num(); ++i)
		{
			function(m_resourceCSinkEntityIds[i]);
		}
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

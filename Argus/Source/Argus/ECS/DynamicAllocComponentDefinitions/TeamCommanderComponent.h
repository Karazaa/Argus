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
	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> > m_seenResourceASourceExtractionData;

	ARGUS_IGNORE()
	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> > m_seenResourceBSourceExtractionData;

	ARGUS_IGNORE()
	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> > m_seenResourceCSourceExtractionData;

	ARGUS_IGNORE()
	TArray<TeamCommanderPriority, ArgusContainerAllocator<11u> > m_priorities;

	ARGUS_IGNORE()
	TBitArray<ArgusContainerAllocator<0u> > m_revealedAreas;

	float m_revealedAreaDimensionLength = 800.0f;

	ARGUS_IGNORE()
	ETeam m_teamToCommand = ETeam::None;

	void ResetUpdateArrays()
	{
		m_idleEntityIdsForTeam.Reset();
		m_priorities.Reset();
	}

	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> >& GetSeenSourceExtractionDataForResourceType(EResourceType type)
	{
		switch (type)
		{
			case EResourceType::ResourceA:
				return m_seenResourceASourceExtractionData;
			case EResourceType::ResourceB:
				return m_seenResourceBSourceExtractionData;
			case EResourceType::ResourceC:
				return m_seenResourceCSourceExtractionData;
			default:
				break;
		}

		return m_seenResourceASourceExtractionData;
	}

	template <typename Function>
	bool IterateSeenResourceSourcesOfType(EResourceType type, Function&& function)
	{
		TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> >& sources = GetSeenSourceExtractionDataForResourceType(type);
		for (int32 i = 0; i < sources.Num(); ++i)
		{
			if (function(sources[i]))
			{
				return true;
			}
		}

		return false;
	}

	template <typename Function>
	bool IterateAllSeenResourceSources(Function&& function) 
	{
		for (int32 i = 0; i < m_seenResourceASourceExtractionData.Num(); ++i)
		{
			if (function(m_seenResourceASourceExtractionData[i]))
			{
				return true;
			}
		}
		for (int32 i = 0; i < m_seenResourceBSourceExtractionData.Num(); ++i)
		{
			if (function(m_seenResourceBSourceExtractionData[i]))
			{
				return true;
			}
		}
		for (int32 i = 0; i < m_seenResourceCSourceExtractionData.Num(); ++i)
		{
			if (function(m_seenResourceCSourceExtractionData[i]))
			{
				return true;
			}
		}

		return false;
	}

	void AddSeenResourceSourceIfNotPresent(EResourceType type, uint16 entityId)
	{
		TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> >& sources = GetSeenSourceExtractionDataForResourceType(type);
		for (int32 i = 0; i < sources.Num(); ++i)
		{
			if (sources[i].HasSourceEntityId(entityId))
			{
				return;
			}
		}

		ResourceSourceExtractionData& emplacedData = sources.Emplace_GetRef();
		emplacedData.SetSourceEntityId(entityId);
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

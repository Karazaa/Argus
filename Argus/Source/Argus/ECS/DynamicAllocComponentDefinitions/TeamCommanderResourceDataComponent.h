// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/TeamCommanderPriorities.h"

struct TeamCommanderResourceDataComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA
	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> > m_seenResourceASourceExtractionData;

	ARGUS_COMP_NO_DATA
	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> > m_seenResourceBSourceExtractionData;

	ARGUS_COMP_NO_DATA
	TArray<ResourceSourceExtractionData, ArgusContainerAllocator<10u> > m_seenResourceCSourceExtractionData;

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
};

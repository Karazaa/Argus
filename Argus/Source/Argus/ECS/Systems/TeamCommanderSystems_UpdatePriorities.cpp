// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems_UpdatePriorities.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"

void TeamCommanderSystems_UpdatePriorities::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::RunSystems);

	ArgusEntity::IterateTeamEntities(TeamCommanderSystems_UpdatePriorities::UpdateTeamCommanderPriorities);
}

void TeamCommanderSystems_UpdatePriorities::UpdateTeamCommanderPriorities(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateTeamCommanderPriorities);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (uint8 i = 0u; i < static_cast<uint8>(ETeamCommanderDirective::Count); ++i)
	{
		const ETeamCommanderDirective directiveToEvaluate = static_cast<ETeamCommanderDirective>(i);
		switch (directiveToEvaluate)
		{
			case ETeamCommanderDirective::ConstructResourceSink:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_entityCategory.m_entityCategoryType = EEntityCategoryType::ResourceSink;
					priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(j);
					UpdateConstructResourceSinkTeamPriority(teamCommanderComponent, priority);
				}
				continue;
			case ETeamCommanderDirective::ExtractResources:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_entityCategory.m_entityCategoryType = EEntityCategoryType::ResourceSource;
					priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(j);
					UpdateResourceExtractionTeamPriority(teamCommanderComponent, priority);
				}
				continue;
			case ETeamCommanderDirective::SpawnUnit:
				for (uint8 j = 0u; j < static_cast<uint8>(EEntityCategoryType::Count); ++j)
				{
					EEntityCategoryType unitCategoryType = static_cast<EEntityCategoryType>(j);
					if (unitCategoryType == EEntityCategoryType::ResourceSink || unitCategoryType == EEntityCategoryType::ResourceSource)
					{
						continue;
					}

					if (unitCategoryType == EEntityCategoryType::Extractor)
					{
						for (uint8 k = 0u; k < static_cast<uint8>(EResourceType::Count); ++k)
						{
							TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
							priority.m_directive = directiveToEvaluate;
							priority.m_entityCategory.m_entityCategoryType = unitCategoryType;
							priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(k);
							UpdateSpawnUnitTeamPriority(teamCommanderComponent, priority);
						}
					}
					else
					{
						TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
						priority.m_directive = directiveToEvaluate;
						priority.m_entityCategory.m_entityCategoryType = unitCategoryType;
						priority.m_entityCategory.m_resourceType = EResourceType::Count;
						UpdateSpawnUnitTeamPriority(teamCommanderComponent, priority);
					}
				}
				continue;
			case ETeamCommanderDirective::Scout:
			{
				TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
				priority.m_directive = directiveToEvaluate;
				UpdateScoutingTeamPriority(teamCommanderComponent, priority);
				continue;
			}
			default:
				continue;
		}
	}

	teamCommanderComponent->m_priorities.Sort();
}

void TeamCommanderSystems_UpdatePriorities::UpdateConstructResourceSinkTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateConstructResourceSinkTeamPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ConstructResourceSink || priority.m_entityCategory.m_resourceType == EResourceType::Count)
	{
		return;
	}

	bool updated = teamCommanderComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId != ArgusECSConstants::k_maxEntities && data.m_resourceSinkEntityId == ArgusECSConstants::k_maxEntities)
		{
			priority.m_weight = 2.0f;
			return true;
		}

		return false;
	});

	if (updated)
	{
		return;
	}

	priority.m_weight = 0.0f;
}

void TeamCommanderSystems_UpdatePriorities::UpdateResourceExtractionTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateResourceExtractionTeamPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ExtractResources)
	{
		return;
	}

	bool updated = teamCommanderComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId != ArgusECSConstants::k_maxEntities && data.m_resourceExtractorEntityId == ArgusECSConstants::k_maxEntities)
		{
			priority.m_weight = 1.0f;
			return true;
		}

		return false;
	});

	if (updated)
	{
		return;
	}

	priority.m_weight = 0.0f;
}

void TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitTeamPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::SpawnUnit)
	{
		return;
	}

	if (priority.m_entityCategory.m_entityCategoryType == EEntityCategoryType::Carrier)
	{
		priority.m_weight = -0.5f;
		return;
	}

	float extractorNeedWeight = -0.5f;
	teamCommanderComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority, &extractorNeedWeight](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId != ArgusECSConstants::k_maxEntities && data.m_resourceExtractorEntityId == ArgusECSConstants::k_maxEntities && data.m_resourceSinkConstructorEntityId == ArgusECSConstants::k_maxEntities)
		{
			extractorNeedWeight += 1.0f;
		}

		return false;
	});

	for (int32 i = 0; i < teamCommanderComponent->m_spawningEntityRecordIds.Num(); ++i)
	{
		const UArgusActorRecord* argusActorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(teamCommanderComponent->m_spawningEntityRecordIds[i]);
		if (!argusActorRecord)
		{
			continue;
		}

		const UArgusEntityTemplate* entityTemplate = argusActorRecord->m_entityTemplate.LoadAndStorePtr();
		if (!entityTemplate)
		{
			continue;
		}

		if (entityTemplate->DoesTemplateSatisfyEntityCategory(priority.m_entityCategory))
		{
			extractorNeedWeight -= 1.0f;
		}
	}

	for (int32 i = 0; i < teamCommanderComponent->m_idleEntityIdsForTeam.Num(); ++i)
	{
		ArgusEntity entity = ArgusEntity::RetrieveEntity(teamCommanderComponent->m_idleEntityIdsForTeam[i]);
		if (!entity)
		{
			continue;
		}

		if (entity.DoesEntitySatisfyEntityCategory(priority.m_entityCategory))
		{
			extractorNeedWeight -= 1.0f;
		}
	}

	priority.m_weight = extractorNeedWeight;
}

void TeamCommanderSystems_UpdatePriorities::UpdateScoutingTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateScoutingTeamPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::Scout)
	{
		return;
	}

	// TODO JAMES: We should probably massively de-prioritize this based on how much of the map has been discovered. 

	priority.m_weight = 1.0f;
}

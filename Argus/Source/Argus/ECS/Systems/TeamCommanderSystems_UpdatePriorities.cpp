// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems_UpdatePriorities.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"

void TeamCommanderSystems_UpdatePriorities::RunSystems()
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::RunSystems);

	ArgusIterators::IterateTeamEntities(TeamCommanderSystems_UpdatePriorities::UpdateTeamCommanderPriorities);
}

void TeamCommanderSystems_UpdatePriorities::UpdateTeamCommanderPriorities(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateTeamCommanderPriorities);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent = teamEntity.GetComponent<TeamCommanderResourceDataComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderResourceDataComponent, ArgusECSLog);

	for (uint8 i = 0u; i < static_cast<uint8>(ETeamCommanderDirective::Count); ++i)
	{
		const ETeamCommanderDirective directiveToEvaluate = static_cast<ETeamCommanderDirective>(i);
		switch (directiveToEvaluate)
		{
			case ETeamCommanderDirective::StartConstruction:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_entityCategory.m_entityCategoryType = EEntityCategoryType::ResourceSink;
					priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(j);
					UpdateConstructResourceSinkTeamPriority(teamCommanderResourceDataComponent, priority);
				}
				continue;
			case ETeamCommanderDirective::ContinueConstruction:
			{
				TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
				priority.m_directive = directiveToEvaluate;
				UpdateContinueConstructionPriority(teamCommanderComponent, priority);
				continue;
			}
			case ETeamCommanderDirective::ExtractResources:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_entityCategory.m_entityCategoryType = EEntityCategoryType::ResourceSource;
					priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(j);
					UpdateResourceExtractionTeamPriority(teamCommanderResourceDataComponent, priority);
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
							UpdateSpawnUnitTeamPriority(teamCommanderComponent, teamCommanderResourceDataComponent, priority);
						}
					}
					else
					{
						TeamCommanderPriority& priority = teamCommanderComponent->m_priorities.Emplace_GetRef();
						priority.m_directive = directiveToEvaluate;
						priority.m_entityCategory.m_entityCategoryType = unitCategoryType;
						UpdateSpawnUnitTeamPriority(teamCommanderComponent, teamCommanderResourceDataComponent, priority);
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

void TeamCommanderSystems_UpdatePriorities::UpdateConstructResourceSinkTeamPriority(TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateConstructResourceSinkTeamPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderResourceDataComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::StartConstruction || priority.m_entityCategory.m_resourceType == EResourceType::Count)
	{
		return;
	}

	bool updated = teamCommanderResourceDataComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
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

void TeamCommanderSystems_UpdatePriorities::UpdateResourceExtractionTeamPriority(TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateResourceExtractionTeamPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderResourceDataComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ExtractResources)
	{
		return;
	}

	bool updated = teamCommanderResourceDataComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId != ArgusECSConstants::k_maxEntities && 
			data.m_resourceExtractorEntityId == ArgusECSConstants::k_maxEntities &&
			data.m_resourceSinkEntityId != ArgusECSConstants::k_maxEntities)
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

void TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitTeamPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(teamCommanderResourceDataComponent, ArgusECSLog);
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
	teamCommanderResourceDataComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&extractorNeedWeight](const ResourceSourceExtractionData& data)
	{
		if (data.m_resourceSourceEntityId == ArgusECSConstants::k_maxEntities || data.m_resourceExtractorEntityId != ArgusECSConstants::k_maxEntities)
		{
			return false;
		}

		if (data.m_resourceSinkEntityId != ArgusECSConstants::k_maxEntities)
		{
			ArgusEntity sinkEntity = ArgusEntity::RetrieveEntity(data.m_resourceSinkEntityId);
			if (sinkEntity.IsUnderConstruction())
			{
				return false;
			}
		}

		extractorNeedWeight += 1.0f;
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

void TeamCommanderSystems_UpdatePriorities::UpdateContinueConstructionPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateContinueConstructionPriority);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (priority.m_directive != ETeamCommanderDirective::ContinueConstruction)
	{
		return;
	}

	for (const TPair<uint16, ConstructionData>& pair : teamCommanderComponent->m_inProgressConstructionData)
	{
		if (pair.Value.m_beingConstructedEntityId != ArgusECSConstants::k_maxEntities && pair.Value.m_constructingOtherEntityId == ArgusECSConstants::k_maxEntities)
		{
			priority.m_weight += 1.0f;
		}
	}
}

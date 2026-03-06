// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems_GatherInfo.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TeamCommanderSystems.h"

void TeamCommanderSystems_GatherInfo::RunSystems()
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::RunSystems);

	ArgusEntity::IterateTeamEntities(TeamCommanderSystems_GatherInfo::ClearUpdatesPerCommanderEntity);
	ArgusEntity::IterateSystemsArgs<TeamCommanderSystemsArgs>(TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntity);
}

void TeamCommanderSystems_GatherInfo::ClearUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::ClearUpdatesPerCommanderEntity);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	teamCommanderComponent->IterateAllSeenResourceSources([](ResourceSourceExtractionData& data)
	{
		ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity::RetrieveEntity(data.m_resourceSinkEntityId), data);
		ClearResourceSinkConstructorFromExtractionDataIfNeeded(ArgusEntity::RetrieveEntity(data.m_resourceSinkConstructorEntityId), data);
		ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity::RetrieveEntity(data.m_resourceExtractorEntityId), data);
		return false;
	});
	teamCommanderComponent->ResetUpdateArrays();
}

void TeamCommanderSystems_GatherInfo::ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity existingResourceSinkEntity, ResourceSourceExtractionData& data)
{
	if (!existingResourceSinkEntity)
	{
		data.m_resourceSinkEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!existingResourceSinkEntity.IsAlive())
	{
		data.m_resourceSinkEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}
}

void TeamCommanderSystems_GatherInfo::ClearResourceSinkConstructorFromExtractionDataIfNeeded(ArgusEntity existingResourceSinkConstructorEntity, ResourceSourceExtractionData& data)
{
	if (!existingResourceSinkConstructorEntity)
	{
		data.m_resourceSinkConstructorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!existingResourceSinkConstructorEntity.IsAlive())
	{
		data.m_resourceSinkConstructorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	const TaskComponent* constructorTaskComponent = existingResourceSinkConstructorEntity.GetComponent<TaskComponent>();
	const TargetingComponent* constructorTargetingComponent = existingResourceSinkConstructorEntity.GetComponent<TargetingComponent>();
	if (!constructorTaskComponent || !constructorTargetingComponent)
	{
		data.m_resourceSinkConstructorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (constructorTaskComponent->m_constructionState != EConstructionState::DispatchedToConstructOther && constructorTaskComponent->m_constructionState != EConstructionState::ConstructingOther)
	{
		data.m_resourceSinkConstructorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!constructorTargetingComponent->HasEntityTarget())
	{
		data.m_resourceSinkConstructorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (data.m_resourceSinkEntityId != ArgusECSConstants::k_maxEntities && constructorTargetingComponent->m_targetEntityId != data.m_resourceSinkEntityId)
	{
		data.m_resourceSinkConstructorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}
}

void TeamCommanderSystems_GatherInfo::ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity existingResourceExtractorEntity, ResourceSourceExtractionData& data)
{
	if (!existingResourceExtractorEntity)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!existingResourceExtractorEntity.IsAlive())
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	const TaskComponent* extractorTaskComponent = existingResourceExtractorEntity.GetComponent<TaskComponent>();
	const TargetingComponent* extractorTargetingComponent = existingResourceExtractorEntity.GetComponent<TargetingComponent>();
	if (!extractorTaskComponent || !extractorTargetingComponent)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (extractorTaskComponent->m_directiveFromTeamCommander != ETeamCommanderDirective::ExtractResources)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (!extractorTargetingComponent->HasEntityTarget())
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	if (extractorTargetingComponent->m_targetEntityId != data.m_resourceSourceEntityId && extractorTargetingComponent->m_targetEntityId != data.m_resourceSinkEntityId)
	{
		data.m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}
}

void TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_identityComponent->m_team == ETeam::None)
	{
		ArgusEntity::IterateTeamEntities([&components](ArgusEntity teamCommanderEntity)
		{
			TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerNeutralEntity(components, teamCommanderEntity);
		});
		return;
	}

	TeamCommanderComponent* teamCommanderComponent = ArgusEntity::GetTeamEntity(components.m_identityComponent->m_team).GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	UpdateTeamCommanderPerEntityOnTeam(components, teamCommanderComponent);
}

void TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntityOnTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_entity.IsAlive() && components.m_entity.IsIdle() && !components.m_entity.IsPassenger())
	{
		teamCommanderComponent->m_idleEntityIdsForTeam.Add(components.m_entity.GetId());
	}

	UpdateResourceExtractionDataPerSink(components, teamCommanderComponent);
	UpdateRevealedAreasPerEntityOnTeam(components, teamCommanderComponent);
	UpdateSpawningUnitTypesPerSpawner(components, teamCommanderComponent);
}

void TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerNeutralEntity);

	TeamCommanderComponent* teamCommanderComponent = teamCommanderEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (ResourceComponent* resourceComponent = components.m_entity.GetComponent<ResourceComponent>())
	{
		if (resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Source && components.m_identityComponent->WasEverSeenBy(teamCommanderComponent->m_teamToCommand))
		{
			for (uint8 i = 0u; i < static_cast<uint8>(EResourceType::Count); ++i)
			{
				EResourceType type = static_cast<EResourceType>(i);
				if (components.m_resourceComponent->m_currentResources.HasResourceType(type))
				{
					teamCommanderComponent->AddSeenResourceSourceIfNotPresent(type, components.m_entity.GetId());
				}
			}
		}
	}
}

void TeamCommanderSystems_GatherInfo::UpdateResourceExtractionDataPerSink(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateResourceExtractionDataPerSink);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_entity.IsAlive() || !components.m_resourceComponent || components.m_resourceComponent->m_resourceComponentOwnerType != EResourceComponentOwnerType::Sink)
	{
		return;
	}

	for (uint8 i = 0u; i < static_cast<uint8>(EResourceType::Count); ++i)
	{
		EResourceType type = static_cast<EResourceType>(i);
		if (!components.m_resourceComponent->m_currentResources.HasResourceType(type))
		{
			continue;
		}

		teamCommanderComponent->IterateSeenResourceSourcesOfType(type, [&components](ResourceSourceExtractionData& data)
		{
			if (data.m_resourceSourceEntityId == ArgusECSConstants::k_maxEntities)
			{
				return false;
			}

			if (data.m_resourceSinkEntityId != ArgusECSConstants::k_maxEntities)
			{
				return false;
			}

			ArgusEntity sourceEntity = ArgusEntity::RetrieveEntity(data.m_resourceSourceEntityId);
			if (TargetingSystems::IsInSightRangeOfOtherEntity(components.m_entity, sourceEntity))
			{
				data.m_resourceSinkEntityId = components.m_entity.GetId();
			}

			return false;
		});
	}
}

void TeamCommanderSystems_GatherInfo::UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateRevealedAreasPerEntityOnTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_transformComponent || !components.m_targetingComponent)
	{
		return;
	}

	const int32 areaIndex = TeamCommanderSystems::GetAreaIndexFromWorldSpaceLocation(components, teamCommanderComponent);
	if (areaIndex >= 0)
	{
		teamCommanderComponent->m_revealedAreas[areaIndex] = true;
	}
}

void TeamCommanderSystems_GatherInfo::UpdateSpawningUnitTypesPerSpawner(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateSpawningUnitTypesPerSpawner);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_spawningState == ESpawningState::None)
	{
		return;
	}

	const SpawningComponent* spawningComponent = components.m_entity.GetComponent<SpawningComponent>();
	if (!spawningComponent || spawningComponent->m_spawnQueue.IsEmpty())
	{
		return;
	}

	const SpawnEntityInfo& spawnInfo = spawningComponent->m_spawnQueue.First();
	teamCommanderComponent->m_spawningEntityRecordIds.Add(spawnInfo.m_argusActorRecordId);
}

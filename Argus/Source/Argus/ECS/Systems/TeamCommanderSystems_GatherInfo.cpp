// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems_GatherInfo.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/CombatSystems.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TeamCommanderSystems.h"

void TeamCommanderSystems_GatherInfo::RunSystems()
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::RunSystems);

	ArgusIterators::IterateTeamEntitiesParallel(TeamCommanderSystems_GatherInfo::ClearUpdatesPerCommanderEntity);
	ArgusIterators::IterateSystemsArgsByTeamParallel<TeamCommanderSystemsArgs>(TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntity);
}

void TeamCommanderSystems_GatherInfo::ClearUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::ClearUpdatesPerCommanderEntity);

	TeamCommanderComponentCollection components;
	components.PopulateArguments(teamEntity);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	components.m_resourceDataComponent->IterateAllSeenResourceSources([](ResourceSourceExtractionData& data)
	{
		ClearResourceSinkFromExtractionDataIfNeeded(ArgusEntity::RetrieveEntity(data.m_resourceSinkEntityId), data);
		ClearResourceExtractorFromExtractionDataIfNeeded(ArgusEntity::RetrieveEntity(data.m_resourceExtractorEntityId), data);
		return false;
	});
	components.m_baseComponent->ResetUpdateArrays();
	components.m_combatDataComponent->ClearTeamCountArrays();
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
		ArgusIterators::IterateTeamEntities([&components](ArgusEntity teamCommanderEntity)
		{
			TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerNeutralEntity(components, teamCommanderEntity);
		});
		return;
	}

	UpdateTeamCommanderPerEntityOnTeam(components, ArgusEntity::GetTeamEntity(components.m_identityComponent->m_team));
	UpdateSeenByTeamCommandersPerEntity(components);
}

void TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerEntityOnTeam);

	TeamCommanderComponentCollection teamCommanderComponents;
	teamCommanderComponents.PopulateArguments(teamCommanderEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_entity.IsAlive() && components.m_entity.IsIdle() && !components.m_entity.IsPassenger())
	{
		teamCommanderComponents.m_baseComponent->m_idleEntityIdsForTeam.Add(components.m_entity.GetId());
	}

	UpdateResourceExtractionDataPerSink(components, teamCommanderComponents);
	UpdateRevealedAreasPerEntityOnTeam(components, teamCommanderComponents);
	UpdateCombatDataPerEntity(components, teamCommanderComponents);
	UpdateSpawningUnitTypesPerSpawner(components, teamCommanderComponents);
	UpdateConstructionDataPerConstructee(components, teamCommanderComponents);
	UpdateTeamAvailableAbilityIdsPerEntity(components, teamCommanderComponents);
}

void TeamCommanderSystems_GatherInfo::UpdateSeenByTeamCommandersPerEntity(const TeamCommanderSystemsArgs& components)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateSeenByTeamCommandersPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	ArgusIterators::IterateTeamEntitiesInBitmask(components.m_identityComponent->m_everSeenBy, [&components](ArgusEntity teamCommanderEntity)
	{	
		TeamCommanderComponentCollection teamCommanderComponents;
		teamCommanderComponents.PopulateArguments(teamCommanderEntity);
		if (!teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
		{
			return;
		}

		if (components.m_identityComponent->m_team == teamCommanderComponents.m_baseComponent->m_teamToCommand)
		{
			return;
		}

		UpdateSeenByTeamCommanderPerEntity(components, teamCommanderComponents);
	});
}

void TeamCommanderSystems_GatherInfo::UpdateSeenByTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateSeenByTeamCommanderPerEntity);

	UpdateCombatDataPerEntity(components, teamCommanderComponents);
}

void TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateTeamCommanderPerNeutralEntity);

	TeamCommanderComponent* teamCommanderComponent = teamCommanderEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent = teamCommanderEntity.GetComponent<TeamCommanderResourceDataComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderResourceDataComponent, ArgusECSLog);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_resourceComponent)
	{
		if (components.m_resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Source && components.m_identityComponent->WasEverSeenBy(teamCommanderComponent->m_teamToCommand))
		{
			for (uint8 i = 0u; i < static_cast<uint8>(EResourceType::Count); ++i)
			{
				EResourceType type = static_cast<EResourceType>(i);
				if (components.m_resourceComponent->m_currentResources.HasResourceType(type))
				{
					teamCommanderResourceDataComponent->AddSeenResourceSourceIfNotPresent(type, components.m_entity.GetId());
				}
			}
		}
	}
}

void TeamCommanderSystems_GatherInfo::UpdateResourceExtractionDataPerSink(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateResourceExtractionDataPerSink);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
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

		teamCommanderComponents.m_resourceDataComponent->IterateSeenResourceSourcesOfType(type, [&components](ResourceSourceExtractionData& data)
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

void TeamCommanderSystems_GatherInfo::UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateRevealedAreasPerEntityOnTeam);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME) ||
		!components.m_transformComponent || !components.m_targetingComponent)
	{
		return;
	}

	const int32 areaIndex = TeamCommanderSystems::GetAreaIndexFromWorldSpaceLocation(components, teamCommanderComponents.m_baseComponent);
	if (areaIndex >= 0)
	{
		teamCommanderComponents.m_baseComponent->m_revealedAreas[areaIndex] = true;
	}
}

void TeamCommanderSystems_GatherInfo::UpdateCombatDataPerEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateCombatDataPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	EntityCategory combatantCategory;
	combatantCategory.m_entityCategoryType = EEntityCategoryType::Combatant;
	if (!components.m_entity.DoesEntitySatisfyEntityCategory(combatantCategory))
	{
		return;
	}

	const ETeam team = components.m_identityComponent->m_team;
	CombatSystemsArgs combatArgs;
	if (combatArgs.PopulateArguments(components.m_entity))
	{
		if (CombatSystems::CanAttackFlying(combatArgs))
		{
			teamCommanderComponents.m_combatDataComponent->IncrementCanAttackFlyingCombatants(team);
		}

		if (CombatSystems::CanAttackGrounded(combatArgs))
		{
			teamCommanderComponents.m_combatDataComponent->IncrementCanAttackGroundedCombatants(team);
		}
	}

	if (components.m_entity.IsFlying())
	{
		teamCommanderComponents.m_combatDataComponent->IncrementFlyingCombatants(team);
	}
	else
	{
		teamCommanderComponents.m_combatDataComponent->IncrementGroundedCombatants(team);
	}
}

void TeamCommanderSystems_GatherInfo::UpdateSpawningUnitTypesPerSpawner(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateSpawningUnitTypesPerSpawner);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
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
	teamCommanderComponents.m_baseComponent->m_spawningEntityRecordIds.Add(spawnInfo.m_argusActorRecordId);
}

void TeamCommanderSystems_GatherInfo::UpdateConstructionDataPerConstructee(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateConstructionDataPerConstructee);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const uint16 entityId = components.m_entity.GetId();
	if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed && !teamCommanderComponents.m_baseComponent->m_inProgressConstructionData.Contains(entityId))
	{
		ConstructionData& newConstructionData = teamCommanderComponents.m_baseComponent->m_inProgressConstructionData.Emplace(entityId);
		newConstructionData.m_beingConstructedEntityId = entityId;
		return;
	}

	if (components.m_taskComponent->m_constructionState != EConstructionState::DispatchedToConstructOther && components.m_taskComponent->m_constructionState != EConstructionState::ConstructingOther)
	{
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		return;
	}

	const uint16 targetEntityId = components.m_targetingComponent->m_targetEntityId;
	ConstructionData* existingConstructionData = teamCommanderComponents.m_baseComponent->m_inProgressConstructionData.Find(targetEntityId);
	if (existingConstructionData)
	{
		existingConstructionData->m_constructingOtherEntityId = entityId;
	}
	else
	{
		ConstructionData& newConstructionData = teamCommanderComponents.m_baseComponent->m_inProgressConstructionData.Emplace(targetEntityId);
		newConstructionData.m_beingConstructedEntityId = targetEntityId;
		newConstructionData.m_constructingOtherEntityId = entityId;
	}
}

void TeamCommanderSystems_GatherInfo::UpdateTeamAvailableAbilityIdsPerEntity(const TeamCommanderSystemsArgs& components, const TeamCommanderComponentCollection& teamCommanderComponents)
{
	ARGUS_TRACE(TeamCommanderSystems_GatherInfo::UpdateTeamAvailableAbilityIdsPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !teamCommanderComponents.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_abilityComponent)
	{
		return;
	}

	components.m_abilityComponent->IterateActiveAbilityIds([&teamCommanderComponents](uint32 abilityRecordId, EAbilityIndex abilityIndex)
	{
		if (abilityRecordId > 0u)
		{
			teamCommanderComponents.m_baseComponent->m_availableAbilityRecordIds.Add(abilityRecordId);
		}
	});
}

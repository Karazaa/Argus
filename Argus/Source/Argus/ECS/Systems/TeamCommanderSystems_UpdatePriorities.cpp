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

	TeamCommanderComponentCollection components;
	components.PopulateArguments(teamEntity);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	for (uint8 i = 0u; i < static_cast<uint8>(ETeamCommanderDirective::Count); ++i)
	{
		const ETeamCommanderDirective directiveToEvaluate = static_cast<ETeamCommanderDirective>(i);
		switch (directiveToEvaluate)
		{
			case ETeamCommanderDirective::StartConstruction:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = components.m_baseComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_entityCategory.m_entityCategoryType = EEntityCategoryType::ResourceSink;
					priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(j);
					UpdateConstructResourceSinkTeamPriority(components, priority);
				}
				continue;
			case ETeamCommanderDirective::ContinueConstruction:
			{
				TeamCommanderPriority& priority = components.m_baseComponent->m_priorities.Emplace_GetRef();
				priority.m_directive = directiveToEvaluate;
				UpdateContinueConstructionPriority(components, priority);
				continue;
			}
			case ETeamCommanderDirective::ExtractResources:
				for (uint8 j = 0u; j < static_cast<uint8>(EResourceType::Count); ++j)
				{
					TeamCommanderPriority& priority = components.m_baseComponent->m_priorities.Emplace_GetRef();
					priority.m_directive = directiveToEvaluate;
					priority.m_entityCategory.m_entityCategoryType = EEntityCategoryType::ResourceSource;
					priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(j);
					UpdateResourceExtractionTeamPriority(components, priority);
				}
				continue;
			case ETeamCommanderDirective::SpawnUnit:
				for (uint8 j = 0u; j < static_cast<uint8>(EEntityCategoryType::Count); ++j)
				{
					const EEntityCategoryType unitCategoryType = static_cast<EEntityCategoryType>(j);
					if (!EntityCategoryUtils::IsSpawnable(unitCategoryType))
					{
						continue;
					}

					if (unitCategoryType == EEntityCategoryType::Extractor)
					{
						for (uint8 k = 0u; k < static_cast<uint8>(EResourceType::Count); ++k)
						{
							TeamCommanderPriority& priority = components.m_baseComponent->m_priorities.Emplace_GetRef();
							priority.m_directive = directiveToEvaluate;
							priority.m_entityCategory.m_entityCategoryType = unitCategoryType;
							priority.m_entityCategory.m_resourceType = static_cast<EResourceType>(k);
							UpdateSpawnUnitTeamPriority(components, priority);
						}
					}
					else if (unitCategoryType == EEntityCategoryType::Combatant)
					{
						TeamCommanderPriority& attackGroundedPriority = components.m_baseComponent->m_priorities.Emplace_GetRef();
						attackGroundedPriority.m_directive = directiveToEvaluate;
						attackGroundedPriority.m_entityCategory.m_entityCategoryType = unitCategoryType;
						attackGroundedPriority.m_entityCategory.m_attackCapability = ERangedAttackCapability::GroundedOnly;
						UpdateSpawnUnitTeamPriority(components, attackGroundedPriority);

						TeamCommanderPriority& attackFlyingPriority = components.m_baseComponent->m_priorities.Emplace_GetRef();
						attackFlyingPriority.m_directive = directiveToEvaluate;
						attackFlyingPriority.m_entityCategory.m_entityCategoryType = unitCategoryType;
						attackFlyingPriority.m_entityCategory.m_attackCapability = ERangedAttackCapability::FlyingOnly;
						UpdateSpawnUnitTeamPriority(components, attackFlyingPriority);
					}
					else
					{
						TeamCommanderPriority& priority = components.m_baseComponent->m_priorities.Emplace_GetRef();
						priority.m_directive = directiveToEvaluate;
						priority.m_entityCategory.m_entityCategoryType = unitCategoryType;
						UpdateSpawnUnitTeamPriority(components, priority);
					}
				}
				continue;
			case ETeamCommanderDirective::Scout:
			{
				TeamCommanderPriority& priority = components.m_baseComponent->m_priorities.Emplace_GetRef();
				priority.m_directive = directiveToEvaluate;
				UpdateScoutingTeamPriority(components, priority);
				continue;
			}
			default:
				continue;
		}
	}

	components.m_baseComponent->m_priorities.Sort();
}

void TeamCommanderSystems_UpdatePriorities::UpdateConstructResourceSinkTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateConstructResourceSinkTeamPriority);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (priority.m_directive != ETeamCommanderDirective::StartConstruction || priority.m_entityCategory.m_resourceType == EResourceType::Count)
	{
		return;
	}

	bool updated = components.m_resourceDataComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
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

void TeamCommanderSystems_UpdatePriorities::UpdateResourceExtractionTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateResourceExtractionTeamPriority);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (priority.m_directive != ETeamCommanderDirective::ExtractResources)
	{
		return;
	}

	bool updated = components.m_resourceDataComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
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

void TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitTeamPriority);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (priority.m_directive != ETeamCommanderDirective::SpawnUnit)
	{
		return;
	}

	priority.m_weight = -0.5f;

	switch (priority.m_entityCategory.m_entityCategoryType)
	{
		case EEntityCategoryType::Extractor:
			UpdateSpawnUnitResourceExtractorTeamPriority(components, priority);
			break;
		case EEntityCategoryType::Combatant:
			UpdateSpawnUnitCombatantTeamPriority(components, priority);
			break;
		default:
			break;
	}

	for (int32 i = 0; i < components.m_baseComponent->m_spawningEntityRecordIds.Num(); ++i)
	{
		const UArgusActorRecord* argusActorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(components.m_baseComponent->m_spawningEntityRecordIds[i]);
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
			priority.m_weight -= 1.0f;
		}
	}
}

void TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitResourceExtractorTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority)
{
	components.m_resourceDataComponent->IterateSeenResourceSourcesOfType(priority.m_entityCategory.m_resourceType, [&priority](const ResourceSourceExtractionData& data)
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

		priority.m_weight += 1.0f;
		return false;
	});

	for (int32 i = 0; i < components.m_baseComponent->m_idleEntityIdsForTeam.Num(); ++i)
	{
		ArgusEntity entity = ArgusEntity::RetrieveEntity(components.m_baseComponent->m_idleEntityIdsForTeam[i]);
		if (!entity)
		{
			continue;
		}

		if (entity.DoesEntitySatisfyEntityCategory(priority.m_entityCategory))
		{
			priority.m_weight -= 1.0f;
		}
	}
}

void TeamCommanderSystems_UpdatePriorities::UpdateSpawnUnitCombatantTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	int16 difference = 0;
	if (priority.m_entityCategory.m_attackCapability == ERangedAttackCapability::FlyingOnly)
	{
		uint16 numEnemyFlyingCombatants = 0u;
		ArgusIterators::IterateTeamIndiciesInBitmask(components.m_baseComponent->m_enemies, [&components, &numEnemyFlyingCombatants](uint8 index)
		{
			numEnemyFlyingCombatants += components.m_combatDataComponent->m_numFlyingCombatants[index];
		});

		uint16 numCanAttackFlying = components.m_combatDataComponent->GetNumCanAttackFlyingCombatants(components.m_baseComponent->m_teamToCommand);
		difference = static_cast<int16>(numEnemyFlyingCombatants) - static_cast<int16>(numCanAttackFlying);
	}
	else
	{
		uint16 numEnemyGroundedCombatants = 0u;
		ArgusIterators::IterateTeamIndiciesInBitmask(components.m_baseComponent->m_enemies, [&components, &numEnemyGroundedCombatants](uint8 index)
		{
			numEnemyGroundedCombatants += components.m_combatDataComponent->m_numGroundedCombatants[index];
		});
		uint16 numCanAttackGrounded = components.m_combatDataComponent->GetNumCanAttackGroundedCombatants(components.m_baseComponent->m_teamToCommand);
		difference = static_cast<int16>(numEnemyGroundedCombatants) - static_cast<int16>(numCanAttackGrounded);
	}

	// TODO JAMES: Do something with difference to result in a priority.
}

void TeamCommanderSystems_UpdatePriorities::UpdateScoutingTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateScoutingTeamPriority);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (priority.m_directive != ETeamCommanderDirective::Scout)
	{
		return;
	}

	// TODO JAMES: We should probably massively de-prioritize this based on how much of the map has been discovered. 

	priority.m_weight = 1.0f;
}

void TeamCommanderSystems_UpdatePriorities::UpdateContinueConstructionPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority)
{
	ARGUS_TRACE(TeamCommanderSystems_UpdatePriorities::UpdateContinueConstructionPriority);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (priority.m_directive != ETeamCommanderDirective::ContinueConstruction)
	{
		return;
	}

	for (const TPair<uint16, ConstructionData>& pair : components.m_baseComponent->m_inProgressConstructionData)
	{
		if (pair.Value.m_beingConstructedEntityId != ArgusECSConstants::k_maxEntities && pair.Value.m_constructingOtherEntityId == ArgusECSConstants::k_maxEntities)
		{
			priority.m_weight += 1.0f;
		}
	}
}

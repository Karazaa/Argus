// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AbilitySystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"
#include "ComponentDependencies/SpawnEntityInfo.h"
#include "DataComponentDefinitions/TransformComponentData.h"
#include "Systems/ResourceSystems.h"

void AbilitySystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(AbilitySystems::RunSystems);

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(singletonEntity));
		return;
	}

	ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>();
	if (!reticleComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ReticleComponent), ARGUS_NAMEOF(singletonEntity));
		return;
	}

	AbilitySystemsArgs components;
	components.m_reticleComponent = reticleComponent;
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(i)))
		{
			continue;
		}

		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			continue;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			continue;
		}

		ProcessAbilityTaskCommands(components);
	}
}

void AbilitySystems::CastAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return;
	}

	if (!ResourceSystems::ApplyTeamResourceChangeIfAffordable(components.m_entity, abilityRecord->m_requiredResourceChangeToCast))
	{
		components.m_taskComponent->m_abilityState = EAbilityState::None;
		return;
	}

	if (abilityRecord->GetRequiresReticle())
	{
		if (components.m_reticleComponent->m_isBlocked || 
			(abilityRecord->GetSingleCastPerReticle() && components.m_reticleComponent->m_wasAbilityCast))
		{
			components.m_taskComponent->m_abilityState = EAbilityState::None;
			return;
		}

		components.m_reticleComponent->m_wasAbilityCast = true;
	}

	switch (abilityRecord->m_abilityType)
	{
		case EAbilityTypes::Spawn:
			CastSpawnAbility(abilityRecord, components, false, abilityRecord->GetRequiresReticle());
			break;

		case EAbilityTypes::Heal:
			CastHealAbility(abilityRecord, components);
			break;

		case EAbilityTypes::Attack:
			CastAttackAbility(abilityRecord, components);
			break;

		case EAbilityTypes::Construct:
			CastSpawnAbility(abilityRecord, components, true, abilityRecord->GetRequiresReticle());
			break;

		case EAbilityTypes::Vacate:
			CastVacateAbility(abilityRecord, components);
			break;

		default:
			return;
	}

	components.m_taskComponent->m_abilityState = EAbilityState::None;
}

void AbilitySystems::PrepReticle(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return;
	}

	if (!abilityRecord->GetRequiresReticle())
	{
		return;
	}

	if (components.m_reticleComponent->m_abilityRecordId == abilityRecord->m_id)
	{
		return;
	}

	components.m_reticleComponent->m_abilityRecordId = abilityRecord->m_id;

	if (abilityRecord->m_abilityType == EAbilityTypes::Construct)
	{
		PrepReticleForConstructAbility(abilityRecord, components);
	}

	components.m_taskComponent->m_abilityState = EAbilityState::None;
}

void AbilitySystems::ProcessAbilityTaskCommands(const AbilitySystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	uint32 abilityId = 0u;
	switch (components.m_taskComponent->m_abilityState)
	{
		case EAbilityState::ProcessCastAbility0Command:
			abilityId = components.m_abilityComponent->m_ability0Id;
			break;

		case EAbilityState::ProcessCastAbility1Command:
			abilityId = components.m_abilityComponent->m_ability1Id;
			break;

		case EAbilityState::ProcessCastAbility2Command:
			abilityId = components.m_abilityComponent->m_ability2Id;
			break;

		case EAbilityState::ProcessCastAbility3Command:
			abilityId = components.m_abilityComponent->m_ability3Id;
			break;

		case EAbilityState::ProcessCastReticleAbility:
			abilityId = components.m_reticleComponent->m_abilityRecordId;
			if (!components.m_reticleComponent->IsReticleEnabled() || !components.m_abilityComponent->HasAbility(abilityId))
			{
				components.m_taskComponent->m_abilityState = EAbilityState::None;
				return;
			}
			break;

		default:
			return;
	}

	if (abilityId == 0u)
	{
		components.m_taskComponent->m_abilityState = EAbilityState::None;
		return;
	}

	const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(abilityId);
	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return;
	}

	if (abilityRecord->GetRequiresReticle() && components.m_taskComponent->m_abilityState != EAbilityState::ProcessCastReticleAbility)
	{
		PrepReticle(abilityRecord, components);
		return;
	}

	CastAbility(abilityRecord, components);
}

void AbilitySystems::CastSpawnAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components, bool needsConstruction, bool atReticle)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return;
	}

	SpawningComponent* spawningComponent = components.m_entity.GetComponent<SpawningComponent>();
	if (!spawningComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to cast a spawn ability without having a %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(SpawningComponent));
		return;
	}

	UArgusActorRecord* argusActorRecord = abilityRecord->m_argusActorRecord.LoadSynchronous();
	if (!argusActorRecord)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UArgusActorRecord*), ARGUS_NAMEOF(UAbilityRecord*));
		return;
	}

	if (spawningComponent->m_currentQueueSize >= spawningComponent->m_maximumQueueSize)
	{
		return;
	}

	if (components.m_taskComponent->m_spawningState == ESpawningState::None)
	{
		components.m_taskComponent->m_spawningState = ESpawningState::ProcessQueuedSpawnEntity;
	}

	SpawnEntityInfo spawnInfo;
	spawnInfo.m_argusActorRecordId = argusActorRecord->m_id;
	spawnInfo.m_spawningAbilityRecordId = abilityRecord->m_id;
	spawnInfo.m_timeToCastSeconds = abilityRecord->m_timeToCastSeconds;
	spawnInfo.m_needsConstruction = needsConstruction;
	if (atReticle)
	{
		spawnInfo.m_spawnLocationOverride = components.m_reticleComponent->m_reticleLocation;
	}
	spawningComponent->m_spawnQueue.Enqueue(spawnInfo);
	spawningComponent->m_currentQueueSize++;
}

void AbilitySystems::CastHealAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{

}

void AbilitySystems::CastAttackAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{

}

void AbilitySystems::CastVacateAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return;
	}

	CarrierComponent* carrierComponent = components.m_entity.GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		return;
	}

	for (int32 i = 0; i < carrierComponent->m_passengerEntityIds.Num(); ++i)
	{
		ArgusEntity passengerEntity = ArgusEntity::RetrieveEntity(carrierComponent->m_passengerEntityIds[i]);
		if (!passengerEntity)
		{
			continue;
		}

		PassengerComponent* passengerComponent = passengerEntity.GetComponent<PassengerComponent>();
		if (!passengerComponent)
		{
			continue;
		}

		passengerComponent->Set_m_carrierEntityId(ArgusECSConstants::k_maxEntities);
	}

	carrierComponent->m_passengerEntityIds.Empty();
}

void AbilitySystems::PrepReticleForConstructAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return;
	}

	const UArgusActorRecord* argusActorRecord = abilityRecord->m_argusActorRecord.LoadSynchronous();
	if (!argusActorRecord)
	{
		return;
	}

	const UArgusEntityTemplate* argusEntityTemplate = argusActorRecord->m_entityTemplateOverride.LoadSynchronous();
	if (!argusEntityTemplate)
	{
		return;
	}

	const UTransformComponentData* transformComponent = argusEntityTemplate->GetComponentFromTemplate<UTransformComponentData>();
	if (!transformComponent)
	{
		return;
	}

	components.m_reticleComponent->m_radius = transformComponent->m_radius;
}

void AbilitySystems::LogAbilityRecordError(const WIDECHAR* functionName)
{
	ARGUS_LOG
	(
		ArgusECSLog,
		Error,
		TEXT("[%s] Could not successfully retrieve %s from %s."),
		functionName,
		ARGUS_NAMEOF(UAbilityRecord*),
		ARGUS_NAMEOF(ArgusStaticData)
	);
}
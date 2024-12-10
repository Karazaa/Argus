// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AbilitySystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"

void AbilitySystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(AbilitySystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		AbilitySystemsComponentArgs components;
		components.m_entity = potentialEntity;
		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_abilityComponent = components.m_entity.GetComponent<AbilityComponent>();

		if (!components.m_taskComponent || !components.m_abilityComponent)
		{
			continue;
		}

		ProcessAbilityTaskCommands(components);
	}
}

bool AbilitySystems::AbilitySystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_abilityComponent && m_taskComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(AbilitySystemsComponentArgs));

	return false;
}

void AbilitySystems::CastAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
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

	switch (abilityRecord->m_abilityType)
	{
		case EAbilityTypes::Spawn:
			CastSpawnAbility(abilityRecord, components);
			break;

		case EAbilityTypes::Heal:
			CastHealAbility(abilityRecord, components);
			break;

		case EAbilityTypes::Attack:
			CastAttackAbility(abilityRecord, components);
			break;
	}

	components.m_taskComponent->m_abilityState = AbilityState::None;
}

void AbilitySystems::ProcessAbilityTaskCommands(const AbilitySystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_abilityState)
	{
		case AbilityState::ProcessCastAbility0Command:
			CastAbility(ArgusStaticData::GetRecord<UAbilityRecord>(components.m_abilityComponent->m_ability0Id), components);
			break;

		case AbilityState::ProcessCastAbility1Command:
			CastAbility(ArgusStaticData::GetRecord<UAbilityRecord>(components.m_abilityComponent->m_ability1Id), components);
			break;

		case AbilityState::ProcessCastAbility2Command:
			CastAbility(ArgusStaticData::GetRecord<UAbilityRecord>(components.m_abilityComponent->m_ability2Id), components);
			break;

		case AbilityState::ProcessCastAbility3Command:
			CastAbility(ArgusStaticData::GetRecord<UAbilityRecord>(components.m_abilityComponent->m_ability3Id), components);
			break;

		default:
			return;
	}
}

void AbilitySystems::CastSpawnAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
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

	// TODO JAMES: We need to figure out a way to not have plain soft pointers as references between static data records.
	// Ideally, these could be ID only.
	UArgusActorRecord* argusActorRecord = abilityRecord->m_argusActorRecordToSpawn.LoadSynchronous();
	if (!argusActorRecord)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UArgusActorRecord*), ARGUS_NAMEOF(UAbilityRecord*));
		return;
	}

	spawningComponent->m_argusActorRecordId = argusActorRecord->m_id;
	if (abilityRecord->m_timeToSpawnSeconds > 0.0f)
	{
		spawningComponent->m_spawnTimerHandle.StartTimer(components.m_entity, abilityRecord->m_timeToSpawnSeconds);
		components.m_taskComponent->m_spawningState = SpawningState::WaitingToSpawnEntity;
	}
	else
	{
		components.m_taskComponent->m_spawningState = SpawningState::SpawningEntity;
	}
}

void AbilitySystems::CastHealAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
{

}

void AbilitySystems::CastAttackAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
{

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
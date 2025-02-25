// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AbilitySystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"
#include "ComponentDependencies/SpawnEntityInfo.h"
#include "DataComponentDefinitions/TransformComponentData.h"

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
		components.m_reticleComponent = reticleComponent;

		if (!components.m_taskComponent || !components.m_abilityComponent)
		{
			continue;
		}

		if (components.m_taskComponent->m_constructionState == ConstructionState::BeingConstructed)
		{
			continue;
		}

		ProcessAbilityTaskCommands(components);
	}
}

bool AbilitySystems::AbilitySystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_abilityComponent && m_taskComponent && m_reticleComponent)
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

	if (abilityRecord->GetRequiresReticle())
	{
		if (components.m_reticleComponent->m_isBlocked || 
			(abilityRecord->GetSingleCastPerReticle() && components.m_reticleComponent->m_wasAbilityCast))
		{
			components.m_taskComponent->m_abilityState = AbilityState::None;
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

		default:
			return;
	}

	components.m_taskComponent->m_abilityState = AbilityState::None;
}

void AbilitySystems::PrepReticle(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
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

	components.m_taskComponent->m_abilityState = AbilityState::None;
}

void AbilitySystems::ProcessAbilityTaskCommands(const AbilitySystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	uint32 abilityId = 0u;
	switch (components.m_taskComponent->m_abilityState)
	{
		case AbilityState::ProcessCastAbility0Command:
			abilityId = components.m_abilityComponent->m_ability0Id;
			break;

		case AbilityState::ProcessCastAbility1Command:
			abilityId = components.m_abilityComponent->m_ability1Id;
			break;

		case AbilityState::ProcessCastAbility2Command:
			abilityId = components.m_abilityComponent->m_ability2Id;
			break;

		case AbilityState::ProcessCastAbility3Command:
			abilityId = components.m_abilityComponent->m_ability3Id;
			break;

		case AbilityState::ProcessCastReticleAbility:
			abilityId = components.m_reticleComponent->m_abilityRecordId;
			if (!components.m_reticleComponent->IsReticleEnabled() || !components.m_abilityComponent->HasAbility(abilityId))
			{
				components.m_taskComponent->m_abilityState = AbilityState::None;
				return;
			}
			break;

		default:
			return;
	}

	if (abilityId == 0u)
	{
		components.m_taskComponent->m_abilityState = AbilityState::None;
		return;
	}

	const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(abilityId);
	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return;
	}

	if (abilityRecord->GetRequiresReticle() && components.m_taskComponent->m_abilityState != AbilityState::ProcessCastReticleAbility)
	{
		PrepReticle(abilityRecord, components);
		return;
	}

	CastAbility(abilityRecord, components);
}

void AbilitySystems::CastSpawnAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components, bool needsConstruction, bool atReticle)
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

	if (components.m_taskComponent->m_spawningState == SpawningState::None)
	{
		components.m_taskComponent->m_spawningState = SpawningState::ProcessQueuedSpawnEntity;
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

void AbilitySystems::CastHealAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
{

}

void AbilitySystems::CastAttackAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
{

}

void AbilitySystems::CastConstructAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
{

}

void AbilitySystems::PrepReticleForConstructAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components)
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
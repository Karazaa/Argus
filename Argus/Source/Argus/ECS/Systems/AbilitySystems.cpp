// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AbilitySystems.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "ComponentDependencies/SpawnEntityInfo.h"
#include "DataComponentDefinitions/TransformComponentData.h"
#include "Systems/ResourceSystems.h"

void AbilitySystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(AbilitySystems::RunSystems);

	ReticleComponent* reticleComponent = ArgusEntity::GetSingletonEntity().GetComponent<ReticleComponent>();
	ARGUS_RETURN_ON_NULL(reticleComponent, ArgusECSLog);

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
			components.m_taskComponent->m_abilityState = EAbilityState::None;
			continue;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			components.m_taskComponent->m_abilityState = EAbilityState::None;
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

	if (!ResourceSystems::CanEntityAffordTeamResourceChange(components.m_entity, abilityRecord->m_requiredResourceChangeToCast))
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

	bool successfulyCast = false;
	switch (abilityRecord->m_abilityType)
	{
		case EAbilityTypes::Spawn:
			successfulyCast = CastSpawnAbility(abilityRecord, components, false, abilityRecord->GetRequiresReticle());
			break;

		case EAbilityTypes::Heal:
			successfulyCast = CastHealAbility(abilityRecord, components);
			break;

		case EAbilityTypes::Attack:
			successfulyCast = CastAttackAbility(abilityRecord, components);
			break;

		case EAbilityTypes::Construct:
			successfulyCast = CastSpawnAbility(abilityRecord, components, true, abilityRecord->GetRequiresReticle());
			break;

		case EAbilityTypes::Vacate:
			successfulyCast = CastVacateAbility(abilityRecord, components);
			break;

		default:
			return;
	}

	if (successfulyCast)
	{
		if (!ResourceSystems::ApplyTeamResourceChangeIfAffordable(components.m_entity, abilityRecord->m_requiredResourceChangeToCast))
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Successfully casted ability id %d, but could not afford it!") ARGUS_FUNCNAME, abilityRecord->m_id);
		}
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

bool AbilitySystems::CastSpawnAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components, bool needsConstruction, bool atReticle)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return false;
	}

	SpawningComponent* spawningComponent = components.m_entity.GetComponent<SpawningComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(spawningComponent, ArgusECSLog);

	if (spawningComponent->m_currentQueueSize >= spawningComponent->m_maximumQueueSize)
	{
		return false;
	}

	if (components.m_taskComponent->m_spawningState == ESpawningState::None)
	{
		components.m_taskComponent->m_spawningState = ESpawningState::ProcessQueuedSpawnEntity;
	}

	SpawnEntityInfo spawnInfo;
	spawnInfo.m_argusActorRecordId = abilityRecord->m_argusActorRecordId;
	spawnInfo.m_spawningAbilityRecordId = abilityRecord->m_id;
	spawnInfo.m_timeToCastSeconds = abilityRecord->m_timeToCastSeconds;
	spawnInfo.m_needsConstruction = needsConstruction;
	if (atReticle)
	{
		spawnInfo.m_spawnLocationOverride = components.m_reticleComponent->m_reticleLocation;
	}
	spawningComponent->m_spawnQueue.PushLast(spawnInfo);
	spawningComponent->m_currentQueueSize++;

	return true;
}

bool AbilitySystems::CastHealAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{
	return true;
}

bool AbilitySystems::CastAttackAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{
	return true;
}

bool AbilitySystems::CastVacateAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (!abilityRecord)
	{
		LogAbilityRecordError(ARGUS_FUNCNAME);
		return false;
	}

	const TransformComponent* transformComponent = components.m_entity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(transformComponent, ArgusECSLog);

	CarrierComponent* carrierComponent = components.m_entity.GetComponent<CarrierComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(carrierComponent, ArgusECSLog);

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

		TaskComponent* passengerTaskComponent = passengerEntity.GetComponent<TaskComponent>(); 
		TargetingComponent* passengerTargetingComponent = passengerEntity.GetComponent<TargetingComponent>();
		TransformComponent* passengerTransformComponent = passengerEntity.GetComponent<TransformComponent>();
		if (!passengerTaskComponent)
		{
			continue;
		}
		passengerTaskComponent->m_movementState = EMovementState::None;

		if (!passengerTargetingComponent)
		{
			continue;
		}
		passengerTargetingComponent->m_targetEntityId = ArgusECSConstants::k_maxEntities;
		passengerTargetingComponent->m_targetLocation.Reset();

		if (!passengerTransformComponent)
		{
			continue;
		}

		float yawAdjustment = (UE_HALF_PI / 4.0f) * static_cast<float>(i);
		if (i % 2)
		{
			yawAdjustment *= -1.0f;
		}
		passengerTransformComponent->m_location = transformComponent->m_location + ((-ArgusMath::GetDirectionFromYaw(transformComponent->GetCurrentYaw() + yawAdjustment)) * transformComponent->m_radius);
	}

	carrierComponent->m_passengerEntityIds.Empty();
	return true;
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

	const UArgusActorRecord* argusActorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(abilityRecord->m_argusActorRecordId);
	if (!argusActorRecord)
	{
		return;
	}

	const UArgusEntityTemplate* argusEntityTemplate = argusActorRecord->m_entityTemplate.LoadAndStorePtr();
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
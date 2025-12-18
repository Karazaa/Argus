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

	ArgusEntity::IterateSystemsArgs<AbilitySystemsArgs>([reticleComponent, deltaTime](AbilitySystemsArgs& components)
	{
		components.m_reticleComponent = reticleComponent;
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			components.m_taskComponent->m_abilityState = EAbilityState::None;
			return;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			components.m_taskComponent->m_abilityState = EAbilityState::None;
			return;
		}

		if (components.m_abilityComponent->m_abilityToRefundId > 0u)
		{
			ProcessAbilityRefundRequests(components);
		}

		ProcessAbilityTaskCommands(components);
	});
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
	for (int32 i = 0; i < abilityRecord->m_abilityEffects.Num(); ++i)
	{
		switch (abilityRecord->m_abilityEffects[i].m_abilityType)
		{
			case EAbilityTypes::Spawn:
				successfulyCast = CastSpawnAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components, false, abilityRecord->GetRequiresReticle());
				break;

			case EAbilityTypes::Heal:
				successfulyCast = CastHealAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components);
				break;

			case EAbilityTypes::Attack:
				successfulyCast = CastAttackAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components);
				break;

			case EAbilityTypes::Construct:
				successfulyCast = CastSpawnAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components, true, abilityRecord->GetRequiresReticle());
				break;

			case EAbilityTypes::Vacate:
				successfulyCast = CastVacateAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components);
				break;

			case EAbilityTypes::TakeOff:
				successfulyCast = CastFlightTransitionAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components, false);
				break;

			case EAbilityTypes::Land:
				successfulyCast = CastFlightTransitionAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components, true);
				break;

			case EAbilityTypes::AddAbilityOverride:
				successfulyCast = CastAbilityOverrideAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components, true);
				break;

			case EAbilityTypes::RemoveAbilityOverride:
				successfulyCast = CastAbilityOverrideAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components, false);
				break;

			default:
				return;
		}
	}

	if (successfulyCast)
	{
		if (!ResourceSystems::ApplyTeamResourceChangeIfAffordable(components.m_entity, abilityRecord->m_requiredResourceChangeToCast))
		{
			ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Successfully casted ability id %d, but could not afford it!"), ARGUS_FUNCNAME, abilityRecord->m_id);
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

	for (int32 i = 0; i < abilityRecord->m_abilityEffects.Num(); ++i)
	{
		if (abilityRecord->m_abilityEffects[i].m_abilityType == EAbilityTypes::Construct)
		{
			PrepReticleForConstructAbility(abilityRecord, abilityRecord->m_abilityEffects[i], components);
			break;
		}
	}

	components.m_taskComponent->m_abilityState = EAbilityState::None;
}

void AbilitySystems::ProcessAbilityRefundRequests(const AbilitySystemsArgs& components)
{
	ARGUS_TRACE(AbilitySystems::ProcessAbilityRefundRequests);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(components.m_abilityComponent->m_abilityToRefundId);
	ARGUS_RETURN_ON_NULL(abilityRecord, ArgusECSLog);

	const FResourceSet refund = -abilityRecord->m_requiredResourceChangeToCast;
	if (!ResourceSystems::ApplyTeamResourceChangeIfAffordable(components.m_entity, refund))
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Successfully refunded ability id %d, but could not afford the refund???"), ARGUS_FUNCNAME, abilityRecord->m_id);
	}

	components.m_abilityComponent->m_abilityToRefundId = 0u;
}

void AbilitySystems::ProcessAbilityTaskCommands(const AbilitySystemsArgs& components)
{
	ARGUS_TRACE(AbilitySystems::ProcessAbilityTaskCommands);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	uint32 abilityId = 0u;
	switch (components.m_taskComponent->m_abilityState)
	{
		case EAbilityState::ProcessCastAbility0Command:
			abilityId = components.m_abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability0);
			break;

		case EAbilityState::ProcessCastAbility1Command:
			abilityId = components.m_abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability1);
			break;

		case EAbilityState::ProcessCastAbility2Command:
			abilityId = components.m_abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability2);
			break;

		case EAbilityState::ProcessCastAbility3Command:
			abilityId = components.m_abilityComponent->GetActiveAbilityId(EAbilityIndex::Ability3);
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

bool AbilitySystems::CastSpawnAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components, bool needsConstruction, bool atReticle)
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

	if (spawningComponent->m_spawnQueue.Num() >= spawningComponent->m_maximumQueueSize)
	{
		return false;
	}

	if (components.m_taskComponent->m_spawningState == ESpawningState::None)
	{
		components.m_taskComponent->m_spawningState = ESpawningState::ProcessQueuedSpawnEntity;
	}

	SpawnEntityInfo spawnInfo;
	spawnInfo.m_argusActorRecordId = abilityEffect.m_argusActorRecordId;
	spawnInfo.m_spawningAbilityRecordId = abilityRecord->m_id;
	spawnInfo.m_timeToCastSeconds = abilityRecord->m_timeToCastSeconds;
	spawnInfo.m_needsConstruction = needsConstruction;
	if (atReticle)
	{
		spawnInfo.m_spawnLocationOverride = components.m_reticleComponent->m_reticleLocation;
	}
	spawningComponent->m_spawnQueue.PushLast(spawnInfo);

	return true;
}

bool AbilitySystems::CastHealAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components)
{
	return true;
}

bool AbilitySystems::CastAttackAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components)
{
	return true;
}

bool AbilitySystems::CastVacateAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components)
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

	carrierComponent->m_passengerEntityIds.Reset();
	return true;
}

bool AbilitySystems::CastFlightTransitionAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components, bool landing)
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

	if (landing)
	{
		// TODO JAMES: Check landing zone?
	}
	
	const TransformComponent* transformComponent = components.m_entity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(transformComponent, ArgusECSLog);

	if (transformComponent->m_flightCapability != EFlightCapability::BothGroundedAndFlying)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Tried to cast a flight transition ability but do not have the required flight capability!"), ARGUS_FUNCNAME);
		return false;
	}

	components.m_taskComponent->Set_m_flightState(landing ? EFlightState::ProcessLandCommand : EFlightState::ProcessTakeOffCommand);
	return true;
}


bool AbilitySystems::CastAbilityOverrideAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components, bool adding)
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

	if (adding)
	{
		components.m_abilityComponent->AddAbilityOverride(abilityEffect.m_abilityRecordId, abilityEffect.m_abilityIndex);
	}
	else
	{
		components.m_abilityComponent->RemoveAbilityOverride(abilityEffect.m_abilityIndex);
	}

	return true;
}

void AbilitySystems::PrepReticleForConstructAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components)
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

	const UArgusActorRecord* argusActorRecord = ArgusStaticData::GetRecord<UArgusActorRecord>(abilityEffect.m_argusActorRecordId);
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
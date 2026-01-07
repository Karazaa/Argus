// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "CombatSystems.h"
#include "Systems/TargetingSystems.h"

void CombatSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(CombatSystems::RunSystems);

	ArgusEntity::IterateSystemsArgs<CombatSystemsArgs>([deltaTime](CombatSystemsArgs& components)
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		ProcessCombatTaskCommands(deltaTime, components);
	});
}

bool CombatSystems::CanEntityAttackOtherEntity(const ArgusEntity& potentialAttacker, const ArgusEntity& potentialVictim)
{
	if (!potentialVictim.IsAlive())
	{
		return false;
	}

	CombatSystemsArgs components;
	if (!components.PopulateArguments(potentialAttacker))
	{
		return false;
	}

	const IdentityComponent* victimIdentityComponent = potentialVictim.GetComponent<IdentityComponent>();
	const TaskComponent* attackerTaskComponent = potentialAttacker.GetComponent<TaskComponent>();

	if (!victimIdentityComponent || !attackerTaskComponent)
	{
		return false;
	}

	if (!victimIdentityComponent->IsInTeamMask(components.m_identityComponent->m_enemies))
	{
		return false;
	}

	if (attackerTaskComponent->m_flightState == EFlightState::TakingOff || attackerTaskComponent->m_flightState == EFlightState::Landing)
	{
		return false;
	}

	if (const TaskComponent* victimTaskComponent = potentialVictim.GetComponent<TaskComponent>())
	{
		if (victimTaskComponent->m_flightState == EFlightState::Grounded)
		{
			return CanAttackGrounded(components);
		}
		if (victimTaskComponent->m_flightState == EFlightState::Flying)
		{
			return CanAttackFlying(components);
		}
	}

	return true;
}

void CombatSystems::ProcessCombatTaskCommands(float deltaTime, const CombatSystemsArgs& components)
{
	ARGUS_TRACE(CombatSystems::ProcessCombatTaskCommands);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_combatState)
	{
		case ECombatState::None:
			break;
		case ECombatState::DispatchedToAttack:
		case ECombatState::Attacking:
			ProcessAttackCommand(deltaTime, components);
			break;
		case ECombatState::OnAttackMove:
			ProcessAttackMoveCommand(deltaTime, components);
			break;
	}
}

void CombatSystems::ProcessAttackCommand(float deltaTime, const CombatSystemsArgs& components)
{
	ARGUS_TRACE(CombatSystems::ProcessAttackCommand);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		components.m_taskComponent->m_combatState = ECombatState::None;
		return;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (!targetEntity)
	{
		return;
	}

	const TransformComponent* targetTransformComponent = targetEntity.GetComponent<TransformComponent>();
	if (!targetTransformComponent)
	{
		return;
	}

	bool inRange = false;
	switch (components.m_combatComponent->m_attackType)
	{
		case EAttackType::Melee:
			inRange = TargetingSystems::IsInMeleeRangeOfOtherEntity(components.m_entity, targetEntity);
			break;
		case EAttackType::Ranged:
			inRange = TargetingSystems::IsInRangedRangeOfOtherEntity(components.m_entity, targetEntity);
			break;
	}

	if (!inRange)
	{
		components.m_taskComponent->m_combatState = ECombatState::DispatchedToAttack;
		return;
	}

	components.m_taskComponent->m_combatState = ECombatState::Attacking;
	if (components.m_combatComponent->m_intervalDurationSeconds > 0.0f)
	{
		PerformTimerAttack(targetEntity, components);
	}
	else
	{
		PerformContinuousAttack(deltaTime, targetEntity, components);
	}
}

void CombatSystems::ProcessAttackMoveCommand(float deltaTime, const CombatSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const NearbyEntitiesComponent* nearbyEntitiesComponent = components.m_entity.GetComponent<NearbyEntitiesComponent>();
	if (!nearbyEntitiesComponent)
	{
		return;
	}

	const bool canAttackGrounded = CanAttackGrounded(components);
	nearbyEntitiesComponent->IterateSeenEntityIds(CanAttackGrounded(components), CanAttackFlying(components), [&components](uint16 targetEntityId) 
	{
		if (!CanEntityAttackOtherEntity(components.m_entity, ArgusEntity::RetrieveEntity(targetEntityId)))
		{
			return false;
		}

		components.m_targetingComponent->SetEntityTarget(targetEntityId);
		components.m_taskComponent->m_combatState = ECombatState::DispatchedToAttack;
		components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
		return true;
	});
}

void CombatSystems::PerformTimerAttack(const ArgusEntity& targetEntity, const CombatSystemsArgs& components)
{
	if (components.m_combatComponent->m_attackTimerHandle.IsTimerTicking(components.m_entity))
	{
		return;
	}

	if (components.m_combatComponent->m_attackTimerHandle.IsTimerComplete(components.m_entity))
	{
		components.m_combatComponent->m_attackTimerHandle.FinishTimerHandling(components.m_entity);
	}

	if (!CanEntityAttackOtherEntity(components.m_entity, targetEntity))
	{
		StopAttackingEntity(components);
	}

	ApplyDamage(components.m_combatComponent->m_baseDamagePerIntervalOrPerSecond, targetEntity, components);

	components.m_combatComponent->m_attackTimerHandle.StartTimer(components.m_entity, components.m_combatComponent->m_intervalDurationSeconds);
}

void CombatSystems::PerformContinuousAttack(float deltaTime, const ArgusEntity& targetEntity, const CombatSystemsArgs& components)
{
	float amountPerTick = components.m_combatComponent->m_baseDamagePerIntervalOrPerSecond * deltaTime;
	uint32 damage = FMath::FloorToInt32(amountPerTick);
	ApplyDamage(damage, targetEntity, components);
}

void CombatSystems::ApplyDamage(uint32 damageAmount, const ArgusEntity& targetEntity, const CombatSystemsArgs& components)
{
	HealthComponent* targetHealthComponent = targetEntity.GetComponent<HealthComponent>();
	if (!targetHealthComponent)
	{
		return;
	}

	// The entity has taken lethal damage. "Kill" the entity.
	if (targetHealthComponent->m_currentHealth <= damageAmount)
	{
		KillEntity(targetEntity, targetHealthComponent);
		StopAttackingEntity(components);
	}
	else
	{
		targetHealthComponent->m_currentHealth -= damageAmount;
	}
}

void CombatSystems::KillEntity(const ArgusEntity& targetEntity, HealthComponent* targetHealthComponent)
{
	if (!targetHealthComponent)
	{
		return;
	}
	targetHealthComponent->m_currentHealth = 0u;

	TaskComponent* targetTaskComponent = targetEntity.GetComponent<TaskComponent>();
	if (!targetTaskComponent)
	{
		return;
	}
	targetTaskComponent->SetToKillState();

	CarrierComponent* targetCarrierComponent = targetEntity.GetComponent<CarrierComponent>();
	if (!targetCarrierComponent)
	{
		return;
	}

	for (int32 i = 0; i < targetCarrierComponent->m_passengerEntityIds.Num(); ++i)
	{
		ArgusEntity passengerEntity = ArgusEntity::RetrieveEntity(targetCarrierComponent->m_passengerEntityIds[i]);
		if (!passengerEntity)
		{
			continue;
		}
		HealthComponent* passengerHealthComponent = passengerEntity.GetComponent<HealthComponent>();
		CombatSystems::KillEntity(passengerEntity, passengerHealthComponent);
	}

	targetCarrierComponent->m_passengerEntityIds.Reset();
}

void CombatSystems::StopAttackingEntity(const CombatSystemsArgs& components)
{
	components.m_targetingComponent->m_targetEntityId = ArgusECSConstants::k_maxEntities;
	components.m_taskComponent->m_combatState = ECombatState::None;
	components.m_taskComponent->m_movementState = EMovementState::None;
}

bool CombatSystems::CanAttackGrounded(const CombatSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	return (components.m_taskComponent->m_flightState == EFlightState::Grounded && components.m_combatComponent->m_attackType == EAttackType::Melee) ||
		(components.m_combatComponent->m_attackType == EAttackType::Ranged && (components.m_combatComponent->m_rangedAttackCapability == ERangedAttackCapability::GroundedOnly || components.m_combatComponent->m_rangedAttackCapability == ERangedAttackCapability::GroundedAndFlying));
}

bool CombatSystems::CanAttackFlying(const CombatSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	return (components.m_taskComponent->m_flightState == EFlightState::Flying && components.m_combatComponent->m_attackType == EAttackType::Melee) ||
		(components.m_combatComponent->m_attackType == EAttackType::Ranged && (components.m_combatComponent->m_rangedAttackCapability == ERangedAttackCapability::FlyingOnly || components.m_combatComponent->m_rangedAttackCapability == ERangedAttackCapability::GroundedAndFlying));
}

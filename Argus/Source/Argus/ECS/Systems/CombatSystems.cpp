// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "CombatSystems.h"
#include "Systems/TargetingSystems.h"

void CombatSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(CombatSystems::RunSystems);

	CombatSystemsArgs components;
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

		ProcessCombatTaskCommands(deltaTime, components);
	}
}

bool CombatSystems::CanEntityAttackOtherEntity(const ArgusEntity& potentialAttacker, const ArgusEntity& potentialVictim)
{
	const IdentityComponent* attackerIdentityComponent = potentialAttacker.GetComponent<IdentityComponent>();
	const IdentityComponent* victimIdentityComponent = potentialVictim.GetComponent<IdentityComponent>();

	if (!attackerIdentityComponent || !victimIdentityComponent)
	{
		return false;
	}

	return !victimIdentityComponent->IsInTeamMask(attackerIdentityComponent->m_allies);
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

	targetHealthComponent->m_isUnderAttack = true;
	// The entity has taken lethal damage. "Kill" the entity.
	if (targetHealthComponent->m_currentHealth <= damageAmount)
	{
		KillEntity(targetEntity, targetHealthComponent);
		OnKilledOtherEntity(components);
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

void CombatSystems::OnKilledOtherEntity(const CombatSystemsArgs& components)
{
	components.m_targetingComponent->m_targetEntityId = ArgusECSConstants::k_maxEntities;
	components.m_taskComponent->m_combatState = ECombatState::None;
	if (components.m_taskComponent->m_movementState == EMovementState::InRangeOfTargetEntity)
	{
		components.m_taskComponent->m_movementState = EMovementState::None;
	}
}

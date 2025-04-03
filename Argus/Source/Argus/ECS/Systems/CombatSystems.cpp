// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "CombatSystems.h"

void CombatSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(CombatSystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i < ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		CombatSystemsComponentArgs components;
		components.m_entity = ArgusEntity::RetrieveEntity(i);
		if (!components.m_entity)
		{
			continue;
		}

		if (components.m_entity.IsKillable() && !components.m_entity.IsAlive())
		{
			continue;
		}

		components.m_combatComponent = components.m_entity.GetComponent<CombatComponent>();
		components.m_identityComponent = components.m_entity.GetComponent<IdentityComponent>();
		components.m_targetingComponent = components.m_entity.GetComponent<TargetingComponent>();
		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_transformComponent = components.m_entity.GetComponent<TransformComponent>();
		if (!components.m_entity || !components.m_combatComponent || !components.m_taskComponent || 
			!components.m_transformComponent || !components.m_identityComponent || !components.m_targetingComponent)
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

bool CombatSystems::CombatSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_identityComponent && m_targetingComponent && m_taskComponent && m_transformComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(CombatSystemsComponentArgs));

	return false;
}

void CombatSystems::ProcessCombatTaskCommands(float deltaTime, const CombatSystemsComponentArgs& components)
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
		case ECombatState::ShouldAttack:
		case ECombatState::Attacking:
			ProcessAttackCommand(deltaTime, components);
			break;
	}
}

void CombatSystems::ProcessAttackCommand(float deltaTime, const CombatSystemsComponentArgs& components)
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

	float neededRange = 0.0f;
	switch (components.m_combatComponent->m_attackType)
	{
		case EAttackType::Melee:
			neededRange = components.m_targetingComponent->m_meleeRange;
			break;
		case EAttackType::Ranged:
			neededRange = components.m_targetingComponent->m_rangedRange;
			break;
	}

	if (FVector::DistSquared(components.m_transformComponent->m_location, targetTransformComponent->m_location) > FMath::Square(neededRange))
	{
		components.m_taskComponent->m_combatState = ECombatState::ShouldAttack;
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

void CombatSystems::PerformTimerAttack(ArgusEntity& targetEntity, const CombatSystemsComponentArgs& components)
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

void CombatSystems::PerformContinuousAttack(float deltaTime, ArgusEntity& targetEntity, const CombatSystemsComponentArgs& components)
{
	float amountPerTick = components.m_combatComponent->m_baseDamagePerIntervalOrPerSecond * deltaTime;
	uint32 damage = FMath::FloorToInt32(amountPerTick);
	ApplyDamage(damage, targetEntity, components);
}

void CombatSystems::ApplyDamage(uint32 damageAmount, ArgusEntity& targetEntity, const CombatSystemsComponentArgs& components)
{
	TaskComponent* targetTaskComponent = targetEntity.GetComponent<TaskComponent>();
	HealthComponent* targetHealthComponent = targetEntity.GetComponent<HealthComponent>();
	if (!targetTaskComponent || !targetHealthComponent)
	{
		return;
	}

	// The entity has taken lethal damage. "Kill" the entity.
	if (targetHealthComponent->m_currentHealth <= damageAmount)
	{
		targetTaskComponent->SetToKillState();
		targetHealthComponent->m_currentHealth = 0u;
		components.m_targetingComponent->m_targetEntityId = ArgusECSConstants::k_maxEntities;
		components.m_taskComponent->m_combatState = ECombatState::None;
	}
	else
	{
		targetHealthComponent->m_currentHealth -= damageAmount;
	}
}
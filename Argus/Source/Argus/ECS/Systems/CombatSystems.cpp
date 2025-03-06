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
		case CombatState::None:
			break;
		case CombatState::Attack:
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
		components.m_taskComponent->m_combatState = CombatState::None;
		return;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (!targetEntity)
	{
		return;
	}

	const TransformComponent* targetTransformComponent = targetEntity.GetComponent<TransformComponent>();
	HealthComponent* targetHealthComponent = targetEntity.GetComponent<HealthComponent>();
	if (!targetTransformComponent || !targetHealthComponent)
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
		return;
	}

	if (components.m_combatComponent->m_intervalDurationSeconds > 0.0f)
	{
		PerformTimerAttack(targetTransformComponent, targetHealthComponent, components);
	}
	else
	{
		PerformContinuousAttack(deltaTime, targetTransformComponent, targetHealthComponent, components);
	}
}

void CombatSystems::PerformTimerAttack(const TransformComponent* targetTransformComponent, HealthComponent* targetHealthComponent, const CombatSystemsComponentArgs& components)
{
	if (components.m_combatComponent->m_attackTimerHandle.IsTimerTicking(components.m_entity))
	{
		return;
	}

	if (components.m_combatComponent->m_attackTimerHandle.IsTimerComplete(components.m_entity))
	{
		components.m_combatComponent->m_attackTimerHandle.FinishTimerHandling(components.m_entity);
	}

	ApplyDamage(components.m_combatComponent->m_baseDamagePerIntervalOrPerSecond, targetTransformComponent, targetHealthComponent, components);

	components.m_combatComponent->m_attackTimerHandle.StartTimer(components.m_entity, components.m_combatComponent->m_intervalDurationSeconds);
}

void CombatSystems::PerformContinuousAttack(float deltaTime, const TransformComponent* targetTransformComponent, HealthComponent* targetHealthComponent, const CombatSystemsComponentArgs& components)
{
	float amountPerTick = components.m_combatComponent->m_baseDamagePerIntervalOrPerSecond * deltaTime;
	uint32 damage = FMath::FloorToInt32(amountPerTick);
	ApplyDamage(damage, targetTransformComponent, targetHealthComponent, components);
}

void CombatSystems::ApplyDamage(uint32 damageAmount, const TransformComponent* targetTransformComponent, HealthComponent* targetHealthComponent, const CombatSystemsComponentArgs& components)
{
	if (targetHealthComponent->m_currentHealth < damageAmount)
	{
		targetHealthComponent->m_currentHealth = 0u;
		components.m_targetingComponent->m_targetEntityId = ArgusECSConstants::k_maxEntities;
		components.m_taskComponent->m_combatState = CombatState::None;
	}
	else
	{
		targetHealthComponent->m_currentHealth -= damageAmount;
	}
}
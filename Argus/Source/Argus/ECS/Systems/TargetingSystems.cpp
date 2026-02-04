// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TargetingSystems.h"
#include "Systems/CombatSystems.h"

TOptional<FVector> TargetingSystems::GetCurrentTargetLocationForEntity(ArgusEntity entity)
{
	const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return NullOpt;
	}

	TOptional<FVector> output = NullOpt;
	if (targetingComponent->HasLocationTarget())
	{
		output = targetingComponent->m_targetLocation.GetValue();
	}
	else if (targetingComponent->HasEntityTarget())
	{
		ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(targetingComponent->m_targetEntityId);
		if (!targetEntity)
		{
			return NullOpt;
		}

		TransformComponent* targetEntityTransformComponent = targetEntity.GetComponent<TransformComponent>();
		if (!targetEntityTransformComponent)
		{
			return NullOpt;
		}

		output = targetEntityTransformComponent->m_location;
	}

	return output;
}

bool TargetingSystems::IsInMeleeRangeOfOtherEntity(ArgusEntity entity, ArgusEntity otherEntity)
{
	if (!entity || !otherEntity)
	{
		return false;
	}

	const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return false;
	}

	return entity.IsInRangeOfOtherEntity(otherEntity, targetingComponent->m_meleeRange);
}

bool TargetingSystems::IsInRangedRangeOfOtherEntity(ArgusEntity entity, ArgusEntity otherEntity)
{
	if (!entity || !otherEntity)
	{
		return false;
	}

	const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return false;
	}

	return entity.IsInRangeOfOtherEntity(otherEntity, targetingComponent->m_rangedRange);
}

bool TargetingSystems::IsInSightRangeOfOtherEntity(ArgusEntity entity, ArgusEntity otherEntity)
{
	if (!entity || !otherEntity)
	{
		return false;
	}

	const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return false;
	}

	return entity.IsInRangeOfOtherEntity(otherEntity, targetingComponent->m_sightRange);
}

float TargetingSystems::GetRangeToUseForOtherEntity(ArgusEntity entity, ArgusEntity otherEntity)
{
	const TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return 0.0f;
	}

	float range = targetingComponent->m_meleeRange;

	if (!CombatSystems::CanEntityAttackOtherEntity(entity, otherEntity))
	{
		return range;
	}

	const CombatComponent* combatComponent = entity.GetComponent<CombatComponent>();
	if (!combatComponent)
	{
		return range;
	}

	switch (combatComponent->m_attackType)
	{
	case EAttackType::Melee:
		return targetingComponent->m_meleeRange;
	case EAttackType::Ranged:
		return targetingComponent->m_rangedRange;
	default:
		break;
	}

	return range;
}

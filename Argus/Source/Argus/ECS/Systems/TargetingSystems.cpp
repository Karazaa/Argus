// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TargetingSystems.h"

TOptional<FVector> TargetingSystems::GetCurrentTargetLocationForEntity(const ArgusEntity& entity)
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

bool TargetingSystems::IsInMeleeRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity)
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

bool TargetingSystems::IsInRangedRangeOfOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity)
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

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TargetingSystems.h"

void TargetingSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TargetingSystems::RunSystems);

	TargetingSystemsArgs components;
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

		// TODO JAMES: Run individual systems per entity below.
	}
}

void TargetingSystems::TargetNearestEntityMatchingTeamMask(uint16 sourceEntityID, uint8 TeamMask, const TargetingSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const FVector fromLocation = components.m_transformComponent->m_location;
	float minDistSquared = FLT_MAX;
	uint16 minDistEntityId = ArgusECSConstants::k_maxEntities;
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		if (i == sourceEntityID)
		{
			continue;
		}

		TransformComponent* otherTransfromComponent = potentialEntity.GetComponent<TransformComponent>();
		if (!otherTransfromComponent)
		{
			continue;
		}

		IdentityComponent* otherIdentityComponent = potentialEntity.GetComponent<IdentityComponent>();
		if (!otherIdentityComponent)
		{
			continue;
		}

		if (!otherIdentityComponent->IsInTeamMask(TeamMask))
		{
			continue;
		}

		const FVector otherLocation = otherTransfromComponent->m_location;
		const float distSquared = FVector::DistSquared(fromLocation, otherLocation);
		if (distSquared < minDistSquared)
		{
			minDistSquared = distSquared;
			minDistEntityId = i;
		}
	}
	components.m_targetingComponent->m_targetEntityId = minDistEntityId;
}

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
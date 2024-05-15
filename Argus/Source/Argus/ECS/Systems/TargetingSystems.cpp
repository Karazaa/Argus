// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TargetingSystems.h"

void TargetingSystems::RunSystems(float deltaTime)
{
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		std::optional<ArgusEntity> potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity.has_value())
		{
			continue;
		}

		TargetingComponent* targetingComponent = potentialEntity->GetComponent<TargetingComponent>();
		if (!targetingComponent)
		{
			continue;
		}

		// TODO JAMES: Run individual systems per entity below.
	}
}

void TargetingSystems::TargetNearestEntityMatchingFactionMask(ArgusEntity sourceEntity, TransformComponent* sourceTransformComponent, uint8 factionMask)
{
	if (!sourceEntity)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid %s passed for %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(sourceEntity));
		return;
	}

	if (!sourceTransformComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null %s passed for entity %d."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent), sourceEntity.GetId());
		return;
	}

	const FVector fromLocation = sourceTransformComponent->m_transform.GetLocation();
	float minDistSquared = FLT_MAX;
	uint16 minDistEntityId = ArgusECSConstants::k_maxEntities;

	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		std::optional<ArgusEntity> potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity.has_value())
		{
			continue;
		}

		if (i == sourceEntity.GetId())
		{
			continue;
		}

		TransformComponent* otherTransfromComponent = potentialEntity->GetComponent<TransformComponent>();
		if (!otherTransfromComponent)
		{
			continue;
		}

		IdentityComponent* otherIdentityComponent = potentialEntity->GetComponent<IdentityComponent>();
		if (!otherIdentityComponent)
		{
			continue;
		}

		if (!otherIdentityComponent->IsInFactionMask(factionMask))
		{
			continue;
		}

		const FVector otherLocation = otherTransfromComponent->m_transform.GetLocation();
		const float distSquared = FVector::DistSquared(fromLocation, otherLocation);
		if (distSquared < minDistSquared)
		{
			minDistSquared = distSquared;
			minDistEntityId = i;
		}
	}

	TargetingComponent* targetingComponent = sourceEntity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		targetingComponent = sourceEntity.AddComponent<TargetingComponent>();
		if (!targetingComponent)
		{
			return;
		}
	}
	targetingComponent->m_targetEntityId = minDistEntityId;
}

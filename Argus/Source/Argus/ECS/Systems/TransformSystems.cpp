// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "../ComponentDefinitions/TargetingComponent.h"
#include "../ComponentDefinitions/TransformComponent.h"

void TransformSystems::RunSystems(const ArgusEntity& entity)
{
	if (TransformComponent* transformComponent = entity.GetComponent<TransformComponent>())
	{
		FindNearestEntityAndTarget(entity, transformComponent);
	}
}

void TransformSystems::FindNearestEntityAndTarget(const ArgusEntity& fromEntity, TransformComponent* fromTransformComponent)
{
	const FVector fromLocation = fromTransformComponent->m_transform.GetLocation();
	float minDistSquared = FLT_MAX;
	ArgusEntity minDistEntity = fromEntity;

	for (uint32 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		std::optional<ArgusEntity> potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity.has_value())
		{
			continue;
		}

		TransformComponent* otherTransfromComponent = potentialEntity->GetComponent<TransformComponent>();
		if (!otherTransfromComponent)
		{
			continue;
		}

		if (i == fromEntity.GetId())
		{
			continue;
		}

		const FVector otherLocation = otherTransfromComponent->m_transform.GetLocation();
		const float distSquared = FVector::DistSquared(fromLocation, otherLocation);
		if (distSquared < minDistSquared)
		{
			minDistSquared = distSquared;
			minDistEntity = potentialEntity.value();
		}
	}

	TargetingComponent* targetingComponent = fromEntity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		targetingComponent = fromEntity.AddComponent<TargetingComponent>();
		if (!targetingComponent)
		{
			return;
		}
	}
	targetingComponent->m_targetEntityId = minDistEntity.GetId();
}

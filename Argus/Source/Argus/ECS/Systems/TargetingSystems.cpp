// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TargetingSystems.h"

void TargetingSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TargetingSystems::RunSystems)

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		TargetingSystemsComponentArgs components;
		components.m_targetingComponent = potentialEntity.GetComponent<TargetingComponent>();
		components.m_transformComponent = potentialEntity.GetComponent<TransformComponent>();

		if (!components.m_targetingComponent || !components.m_transformComponent)
		{
			continue;
		}

		// TODO JAMES: Run individual systems per entity below.
	}
}

bool TargetingSystems::TargetingSystemsComponentArgs::AreComponentsValidCheck() const
{
	if (!m_targetingComponent || !m_transformComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Targeting Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return false;
	}

	return true;
}

void TargetingSystems::TargetNearestEntityMatchingTeamMask(uint16 sourceEntityID, uint8 TeamMask, const TargetingSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	const FVector fromLocation = components.m_transformComponent->m_transform.GetLocation();
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

		const FVector otherLocation = otherTransfromComponent->m_transform.GetLocation();
		const float distSquared = FVector::DistSquared(fromLocation, otherLocation);
		if (distSquared < minDistSquared)
		{
			minDistSquared = distSquared;
			minDistEntityId = i;
		}
	}
	components.m_targetingComponent->m_targetEntityId = minDistEntityId;
}

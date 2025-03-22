// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "TransformComponentData.h"
#include "ArgusStaticData.h"

void UTransformComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	TransformComponent* TransformComponentRef = entity.AddComponent<TransformComponent>();
	TransformComponentRef->m_location = m_location;
	TransformComponentRef->m_avoidanceGroupSourceLocation = m_avoidanceGroupSourceLocation;
	TransformComponentRef->m_currentVelocity = m_currentVelocity;
	TransformComponentRef->m_proposedAvoidanceVelocity = m_proposedAvoidanceVelocity;
	TransformComponentRef->m_smoothedYaw = ArgusMath::ExponentialDecaySmoother<float>(m_smoothedYawDecayConstant);
	TransformComponentRef->m_targetYaw = m_targetYaw;
	TransformComponentRef->m_desiredSpeedUnitsPerSecond = m_desiredSpeedUnitsPerSecond;
	TransformComponentRef->m_radius = m_radius;
	TransformComponentRef->m_height = m_height;
}

bool UTransformComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UTransformComponentData* otherComponentData = Cast<UTransformComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

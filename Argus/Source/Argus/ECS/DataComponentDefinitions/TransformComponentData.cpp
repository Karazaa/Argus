// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "TransformComponentData.h"

void UTransformComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	TransformComponent* TransformComponentRef = entity.AddComponent<TransformComponent>();
	TransformComponentRef->m_transform = m_transform;
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
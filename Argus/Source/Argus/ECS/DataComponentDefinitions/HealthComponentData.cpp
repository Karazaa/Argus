// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "HealthComponentData.h"

void UHealthComponentData::InstantiateComponentForEntity(ArgusEntity& entity)
{
	HealthComponent* healthComponent = entity.AddComponent<HealthComponent>();
	healthComponent->m_health = Health;
}

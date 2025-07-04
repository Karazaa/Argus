// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "HealthComponentData.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"

void UHealthComponentData::InstantiateComponentForEntity(const ArgusEntity& entity) const
{
	HealthComponent* HealthComponentRef = entity.AddComponent<HealthComponent>();
	ARGUS_RETURN_ON_NULL(HealthComponentRef, ArgusECSLog);

	HealthComponentRef->m_currentHealth = m_currentHealth;
	HealthComponentRef->m_maximumHealth = m_maximumHealth;
}

bool UHealthComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UHealthComponentData* otherComponentData = Cast<UHealthComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

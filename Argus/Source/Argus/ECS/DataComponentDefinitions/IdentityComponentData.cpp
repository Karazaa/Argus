// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "IdentityComponentData.h"

void UIdentityComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	IdentityComponent* IdentityComponentRef = entity.AddComponent<IdentityComponent>();
	IdentityComponentRef->m_team = m_team;
	IdentityComponentRef->m_allies = m_allies;
	IdentityComponentRef->m_enemies = m_enemies;
}

bool UIdentityComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UIdentityComponentData* otherComponentData = Cast<UIdentityComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

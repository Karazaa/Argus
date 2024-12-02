// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "AbilityComponentData.h"
#include "ArgusStaticData.h"

void UAbilityComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	AbilityComponent* AbilityComponentRef = entity.AddComponent<AbilityComponent>();
	AbilityComponentRef->m_ability0Id = m_ability0Id.LoadSynchronous() ? m_ability0Id.LoadSynchronous()->m_id : 0u;
	AbilityComponentRef->m_ability1Id = m_ability1Id.LoadSynchronous() ? m_ability1Id.LoadSynchronous()->m_id : 0u;
	AbilityComponentRef->m_ability2Id = m_ability2Id.LoadSynchronous() ? m_ability2Id.LoadSynchronous()->m_id : 0u;
	AbilityComponentRef->m_ability3Id = m_ability3Id.LoadSynchronous() ? m_ability3Id.LoadSynchronous()->m_id : 0u;
}

bool UAbilityComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UAbilityComponentData* otherComponentData = Cast<UAbilityComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

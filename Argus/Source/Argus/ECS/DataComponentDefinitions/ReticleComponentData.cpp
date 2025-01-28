// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ReticleComponentData.h"
#include "ArgusStaticData.h"

void UReticleComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	ReticleComponent* ReticleComponentRef = entity.AddComponent<ReticleComponent>();
	ReticleComponentRef->m_reticleLocation = m_reticleLocation;
	ReticleComponentRef->m_abilityRecordId = m_abilityRecordId;
	ReticleComponentRef->m_isBlocked = m_isBlocked;
}

bool UReticleComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UReticleComponentData* otherComponentData = Cast<UReticleComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

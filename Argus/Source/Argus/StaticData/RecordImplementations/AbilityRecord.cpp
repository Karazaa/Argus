// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/AbilityRecord.h"
#include "ArgusStaticData.h"

void UAbilityRecord::OnAsyncLoaded() const
{
	if (m_abilityIcon)
	{
		m_abilityIcon.AsyncPreLoadAndStorePtr();
	}
	if (m_reticleMaterial)
	{
		m_reticleMaterial.AsyncPreLoadAndStorePtr();
	}
	for (int32 i = 0; i < m_abilityEffects.Num(); ++i)
	{
		switch (m_abilityEffects[i].m_abilityType)
		{
			case EAbilityTypes::Spawn:
			case EAbilityTypes::Construct:
				ArgusStaticData::AsyncPreLoadRecord<UArgusActorRecord>(m_abilityEffects[i].m_argusActorRecordId);
				break;
			case EAbilityTypes::AddAbilityOverride:
				ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(m_abilityEffects[i].m_abilityRecordId);
				break;
			default:
				break;
		}
	}
}
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
	if (m_argusActorRecordId > 0)
	{
		ArgusStaticData::AsyncPreLoadRecord<UArgusActorRecord>(m_argusActorRecordId);
	}
}
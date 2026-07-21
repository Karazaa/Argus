// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/AbilityRecord.h"
#include "ArgusStaticData.h"
#include "Systems/AbilitySystems.h"

void UAbilityRecord::OnAsyncLoaded() const
{
	if (m_abilityIcon)
	{
		m_abilityIcon.AsyncPreLoadAndStorePtr();
	}

	uint32 materialRecordId = m_reticleMaterial.GetId();
	if (materialRecordId > 0)
	{
		ArgusStaticData::AsyncPreLoadRecord<UMaterialRecord>(materialRecordId);
	}

	for (int32 i = 0; i < m_abilityEffects.Num(); ++i)
	{
		switch (m_abilityEffects[i].m_abilityType)
		{
			case EAbilityTypes::Spawn:
			case EAbilityTypes::Construct:
				ArgusStaticData::AsyncPreLoadRecord<UArgusActorRecord>(m_abilityEffects[i].m_argusActorRecordReference.GetId());
				break;
			case EAbilityTypes::AddAbilityOverride:
				ArgusStaticData::AsyncPreLoadRecord<UAbilityRecord>(m_abilityEffects[i].m_abilityRecordReference.GetId());
				break;
			default:
				break;
		}
	}
}

void UAbilityRecord::ResetSoftPtrLoadStores()
{
	m_abilityIcon.ResetHardPtr();
}

bool UAbilityRecord::DoesAbilitySpawnEntityOfCategory(EntityCategory entityCategory) const
{
	ARGUS_TRACE(UAbilityRecord::DoesAbilitySpawnEntityOfCategory);

	bool* isAlreadySatisfied = m_isEntityCategorySpawnedByAbility.Find(entityCategory);
	if (isAlreadySatisfied)
	{
		return *isAlreadySatisfied;
	}

	bool& isSatisfied = m_isEntityCategorySpawnedByAbility.Add(entityCategory);
	isSatisfied = false;

	const UArgusEntityTemplate* entityTemplate = AbilitySystems::GetEntityTemplateForAbility(this);
	if (!entityTemplate)
	{
		return isSatisfied;
	}

	isSatisfied = entityTemplate->DoesTemplateSatisfyEntityCategory(entityCategory);
	return isSatisfied;
}
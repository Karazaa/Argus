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

bool UAbilityRecord::DoesAbilitySpawnEntityOfCategory(FEntityCategory entityCategory) const
{
	ARGUS_TRACE(UAbilityRecord::DoesAbilitySpawnEntityOfCategory);

	bool* isAlreadySatisfied = m_isEntityCategorySpawnedByAbility.Find(entityCategory);
	if (isAlreadySatisfied)
	{
		return *isAlreadySatisfied;
	}

	return false;
}

#if WITH_EDITOR
void UAbilityRecord::UpdateEntityCategoriesSpawnedByAbility()
{
	m_isEntityCategorySpawnedByAbility.Reset();

	const UArgusEntityTemplate* entityTemplate = AbilitySystems::GetEntityTemplateForAbility(this);
	if (!entityTemplate)
	{
		return;
	}
	entityTemplate->CacheComponents();

	ARGUS_LOG(ArgusStaticDataLog, Display, TEXT("===> Updating entity categories spawned by ability for ability, %s"), *GetName())

	const FEntityCategory extractorA = FEntityCategory(EEntityCategoryType::Extractor, EResourceType::ResourceA, ERangedAttackCapability::Count);
	const FEntityCategory extractorB = FEntityCategory(EEntityCategoryType::Extractor, EResourceType::ResourceB, ERangedAttackCapability::Count);
	const FEntityCategory extractorC = FEntityCategory(EEntityCategoryType::Extractor, EResourceType::ResourceC, ERangedAttackCapability::Count);
	const FEntityCategory sinkA = FEntityCategory(EEntityCategoryType::ResourceSink, EResourceType::ResourceA, ERangedAttackCapability::Count);
	const FEntityCategory sinkB = FEntityCategory(EEntityCategoryType::ResourceSink, EResourceType::ResourceB, ERangedAttackCapability::Count);
	const FEntityCategory sinkC = FEntityCategory(EEntityCategoryType::ResourceSink, EResourceType::ResourceC, ERangedAttackCapability::Count);
	const FEntityCategory combatantGrounded = FEntityCategory(EEntityCategoryType::Combatant, EResourceType::Count, ERangedAttackCapability::GroundedOnly);
	const FEntityCategory combatantFlying = FEntityCategory(EEntityCategoryType::Combatant, EResourceType::Count, ERangedAttackCapability::FlyingOnly);

	m_isEntityCategorySpawnedByAbility.Emplace(extractorA, entityTemplate->DoesTemplateSatisfyEntityCategory(extractorA));
	m_isEntityCategorySpawnedByAbility.Emplace(extractorB, entityTemplate->DoesTemplateSatisfyEntityCategory(extractorB));
	m_isEntityCategorySpawnedByAbility.Emplace(extractorC, entityTemplate->DoesTemplateSatisfyEntityCategory(extractorC));
	m_isEntityCategorySpawnedByAbility.Emplace(sinkA, entityTemplate->DoesTemplateSatisfyEntityCategory(sinkA));
	m_isEntityCategorySpawnedByAbility.Emplace(sinkB, entityTemplate->DoesTemplateSatisfyEntityCategory(sinkB));
	m_isEntityCategorySpawnedByAbility.Emplace(sinkC, entityTemplate->DoesTemplateSatisfyEntityCategory(sinkC));
	m_isEntityCategorySpawnedByAbility.Emplace(combatantGrounded, entityTemplate->DoesTemplateSatisfyEntityCategory(combatantGrounded));
	m_isEntityCategorySpawnedByAbility.Emplace(combatantFlying, entityTemplate->DoesTemplateSatisfyEntityCategory(combatantFlying));
}
#endif //WITH_EDITOR
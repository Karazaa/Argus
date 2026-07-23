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

	const EntityCategory extractorA = EntityCategory(EEntityCategoryType::Extractor, EResourceType::ResourceA, ERangedAttackCapability::Count);
	const EntityCategory extractorB = EntityCategory(EEntityCategoryType::Extractor, EResourceType::ResourceB, ERangedAttackCapability::Count);
	const EntityCategory extractorC = EntityCategory(EEntityCategoryType::Extractor, EResourceType::ResourceC, ERangedAttackCapability::Count);
	const EntityCategory sinkA = EntityCategory(EEntityCategoryType::ResourceSink, EResourceType::ResourceA, ERangedAttackCapability::Count);
	const EntityCategory sinkB = EntityCategory(EEntityCategoryType::ResourceSink, EResourceType::ResourceB, ERangedAttackCapability::Count);
	const EntityCategory sinkC = EntityCategory(EEntityCategoryType::ResourceSink, EResourceType::ResourceC, ERangedAttackCapability::Count);
	const EntityCategory combatantGrounded = EntityCategory(EEntityCategoryType::Combatant, EResourceType::Count, ERangedAttackCapability::GroundedOnly);
	const EntityCategory combatantFlying = EntityCategory(EEntityCategoryType::Combatant, EResourceType::Count, ERangedAttackCapability::FlyingOnly);

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
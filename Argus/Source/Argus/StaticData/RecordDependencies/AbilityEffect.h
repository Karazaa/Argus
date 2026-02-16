// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "AbilityTypes.h"
#include "ComponentDefinitions/AbilityComponent.h"
#include "RecordReferences/ArgusActorRecordReference.h"
#include "AbilityEffect.generated.h"

USTRUCT(BlueprintType)
struct FAbilityEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BLueprintReadOnly)
	EAbilityTypes m_abilityType = EAbilityTypes::Heal;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "m_abilityType == EAbilityTypes::Spawn || m_abilityType == EAbilityTypes::Construct", EditConditionHides))
	FUArgusActorRecordReference m_argusActorRecordReference;

	// TODO JAMES: We need some way of record picking this. Right now it is just a plain int property in editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::AddAbilityOverride", EditConditionHides))
	int32 m_abilityRecordId = 0u;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::AddAbilityOverride || m_abilityType == EAbilityTypes::RemoveAbilityOverride", EditConditionHides))
	EAbilityIndex m_abilityIndex = EAbilityIndex::None;
};
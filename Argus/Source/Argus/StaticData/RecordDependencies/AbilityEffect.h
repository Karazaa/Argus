// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "AbilityTypes.h"
#include "ComponentDefinitions/AbilityComponent.h"
#include "AbilityEffect.generated.h"

USTRUCT(BlueprintType)
struct FAbilityEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BLueprintReadOnly)
	EAbilityTypes m_abilityType = EAbilityTypes::Heal;

	// TODO JAMES: We need some way of record picking this. Right now it is just a plain int property in editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::Spawn || m_abilityType == EAbilityTypes::Construct", EditConditionHides))
	int32 m_argusActorRecordId = 0u;

	// TODO JAMES: We need some way of record picking this. Right now it is just a plain int property in editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::AddAbilityOverride", EditConditionHides))
	int32 m_abilityRecordId = 0u;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::AddAbilityOverride || m_abilityType == EAbilityTypes::RemoveAbilityOverride", EditConditionHides))
	EAbilityIndex m_abilityIndex = EAbilityIndex::None;
};
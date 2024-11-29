// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/ArgusActorRecord.h"
#include "RecordDependencies/AbilityTypes.h"
#include "AbilityRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UAbilityRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAbilityTypes m_abilityType = EAbilityTypes::Spawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::Spawn", EditConditionHides))
	TSoftObjectPtr<UArgusActorRecord> m_argusActorRecordToSpawn;
};
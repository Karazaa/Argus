// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/ArgusActorRecord.h"
#include "RecordDependencies/AbilityTypes.h"
#include "AbilityRecord.generated.h"

class UMaterial;

UCLASS(BlueprintType)
class ARGUS_API UAbilityRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float m_timeToCastSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAbilityTypes m_abilityType = EAbilityTypes::Spawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool m_requiresReticle = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::Spawn || m_abilityType == EAbilityTypes::Construct", EditConditionHides))
	TSoftObjectPtr<UArgusActorRecord> m_argusActorRecord;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_requiresReticle", EditConditionHides))
	TSoftObjectPtr<UMaterial> m_reticleMaterial;
};
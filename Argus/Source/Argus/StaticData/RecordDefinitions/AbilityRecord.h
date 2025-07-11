// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ResourceSet.h"
#include "Materials/MaterialInterface.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "RecordDependencies/AbilityTypes.h"
#include "SoftPtrLoadStore.h"
#include "AbilityRecord.generated.h"

class UMaterial;

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EReticleFlags : uint8
{
	None = 0,
	RequiresReticle = 1 << 0,
	SingleCastPerReticle = 1 << 1,
	DisableReticleAfterCast = 1 << 2
};
ENUM_CLASS_FLAGS(EReticleFlags);

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
	FSoftPtrLoadStore_UTexture m_abilityIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FResourceSet m_requiredResourceChangeToCast;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = EReticleFlags))
	uint8 m_reticleFlags = 0u;

	// TODO JAMES: We need some way of record picking this. Right now it is just a plain int property in editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_abilityType == EAbilityTypes::Spawn || m_abilityType == EAbilityTypes::Construct", EditConditionHides))
	int32 m_argusActorRecordId = 0u;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftPtrLoadStore_UMaterialInterface m_reticleMaterial;

	bool GetRequiresReticle() const
	{
		return static_cast<bool>(m_reticleFlags & static_cast<uint8>(EReticleFlags::RequiresReticle));
	}

	bool GetSingleCastPerReticle() const
	{
		return static_cast<bool>(m_reticleFlags & static_cast<uint8>(EReticleFlags::SingleCastPerReticle));
	}

	bool GetDisableReticleAfterCast() const
	{
		return static_cast<bool>(m_reticleFlags & static_cast<uint8>(EReticleFlags::DisableReticleAfterCast));
	}

	virtual void OnAsyncLoaded() const override;
};
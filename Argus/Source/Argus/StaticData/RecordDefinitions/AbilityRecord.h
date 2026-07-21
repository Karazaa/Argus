// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMap.h"
#include "ArgusSetAllocator.h"
#include "ArgusStaticRecord.h"
#include "ComponentDependencies/EntityCategory.h"
#include "RecordDependencies/AbilityEffect.h"
#include "RecordReferences/MaterialRecordReference.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (TitleProperty = ""))
	TArray<FAbilityEffect> m_abilityEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float m_timeToCastSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectLoadStore_UTexture m_abilityIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FResourceSet m_requiredResourceChangeToCast;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/Argus.EReticleFlags"))
	uint8 m_reticleFlags = 0u;

	UPROPERTY(EditAnywhere)
	FUMaterialRecordReference m_reticleMaterial;

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
	virtual void ResetSoftPtrLoadStores() override;

	bool DoesAbilitySpawnEntityOfCategory(EntityCategory entityCategory) const;

private:
	mutable ArgusMap<EntityCategory, bool, ArgusSetAllocator<14u> > m_isEntityCategorySpawnedByAbility;
};
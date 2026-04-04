// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "SoftPtrLoadStore.h"

#include "MaterialRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UMaterialRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectLoadStore_UMaterialInterface m_material;

	void OnAsyncLoaded() const override;
};
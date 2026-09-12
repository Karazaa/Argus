// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "SoftPtrLoadStore.h"
#include "StructuralEntityTemplateRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UStructuralEntityTemplateRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftObjectLoadStore_UArgusEntityTemplate m_entityTemplate;

	void OnAsyncLoaded() const override;
	void ResetSoftPtrLoadStores() override;
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ArgusStaticRecord.generated.h"

UCLASS()
class UArgusStaticRecord : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	uint32 m_id;

#if WITH_EDITOR && !IS_PACKAGING_ARGUS
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif //WITH_EDITOR && !IS_PACKAGING_ARGUS
};
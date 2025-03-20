// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

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

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif //WITH_EDITOR
};
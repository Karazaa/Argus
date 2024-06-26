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
	uint32 m_id;
};
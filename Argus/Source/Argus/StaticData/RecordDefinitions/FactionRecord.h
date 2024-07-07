// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "Math/Color.h"
#include "FactionRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UFactionRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString m_factionName;
};
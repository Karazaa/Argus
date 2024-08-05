// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "Math/Color.h"
#include "TeamColorRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UTeamColorRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FColor m_teamColor;
};
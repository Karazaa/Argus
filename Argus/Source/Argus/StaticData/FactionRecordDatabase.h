// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FactionRecord.h"
#include "FactionRecordDatabase.generated.h"

UCLASS()
class UFactionRecordDatabase : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TMap<uint8, TSoftObjectPtr<UFactionRecord>> m_factionRecords;
};
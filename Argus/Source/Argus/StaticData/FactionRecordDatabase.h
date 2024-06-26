// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/FactionRecord.h"
#include "FactionRecordDatabase.generated.h"

UCLASS()
class UFactionRecordDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	const UFactionRecord* GetRecord(uint8 id) const;

protected:
	UPROPERTY(EditAnywhere)
	TMap<uint8, TSoftObjectPtr<UFactionRecord>> m_factionRecords;
};
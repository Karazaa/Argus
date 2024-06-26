// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/FactionRecord.h"
#include "FactionRecordDatabase.generated.h"

UCLASS()
class UFactionRecordDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	const UFactionRecord* GetRecord(uint32 id) const;
	const uint32 GetIdFromRecordSoftPtr(const TSoftObjectPtr<UFactionRecord>& UFactionRecord) const;

protected:
	UPROPERTY(EditAnywhere)
	TArray<TSoftObjectPtr<UFactionRecord>> m_factionRecords;
};
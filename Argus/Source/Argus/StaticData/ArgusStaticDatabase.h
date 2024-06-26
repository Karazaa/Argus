// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "FactionRecordDatabase.h"
#include "ArgusStaticDatabase.generated.h"

UCLASS()
class UArgusStaticDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	const UFactionRecord* GetUFactionRecord(uint32 id) const;
	const uint32 GetIdFromRecordSoftPtr(const TSoftObjectPtr<UFactionRecord>& UFactionRecord) const;

protected:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UFactionRecordDatabase> m_factionRecordDatabase;
};
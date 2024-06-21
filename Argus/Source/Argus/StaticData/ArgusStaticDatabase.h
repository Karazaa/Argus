// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "FactionRecordDatabase.h"
#include "ArgusStaticDatabase.generated.h"

UCLASS()
class UArgusStaticDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	template<typename ArgusStaticRecord>
	const ArgusStaticRecord* GetRecord(uint8 id) const
	{
		return nullptr;
	}

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UFactionRecordDatabase> m_factionRecordDatabase;

	template<>
	inline const UFactionRecord* GetRecord(uint8 id) const
	{
		UFactionRecordDatabase* factionRecordDatabase = m_factionRecordDatabase.LoadSynchronous();
		if (!factionRecordDatabase)
		{
			return nullptr;
		}

		return factionRecordDatabase->GetRecord(id);
	}
};
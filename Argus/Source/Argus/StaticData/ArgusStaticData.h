// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusGameInstance.h"

class ArgusStaticData
{
public:
	template<typename ArgusStaticRecord>
	static const ArgusStaticRecord* GetRecord(uint32 id)
	{
		return nullptr;
	}

	template<>
	inline const UFactionRecord* GetRecord(uint32 id)
	{
		const UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return nullptr;
		}

		return staticDatabase->GetUFactionRecord(id);
	}

	static const uint32 GetIdFromRecordSoftPtr(const TSoftObjectPtr<UFactionRecord>& UFactionRecord)
	{
		const UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return 0u;
		}

		return staticDatabase->GetIdFromRecordSoftPtr(UFactionRecord);
	}
};
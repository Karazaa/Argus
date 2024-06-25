// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusGameInstance.h"

class ArgusStaticData
{
public:
	template<typename ArgusStaticRecord>
	static const ArgusStaticRecord* GetRecord(uint8 id)
	{
		return nullptr;
	}

	template<>
	inline const UFactionRecord* GetRecord(uint8 id)
	{
		const UArgusStaticDatabase* staticDatabase = UArgusGameInstance::GetStaticDatabase();

		if (!staticDatabase)
		{
			return nullptr;
		}

		return staticDatabase->GetUFactionRecord(id);
	}
};
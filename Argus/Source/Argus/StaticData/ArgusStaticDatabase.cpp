// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusStaticDatabase.h"

const UFactionRecord* UArgusStaticDatabase::GetUFactionRecord(uint8 id) const
{
	UFactionRecordDatabase* factionRecordDatabase = m_factionRecordDatabase.LoadSynchronous();
	if (!factionRecordDatabase)
	{
		UE_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not retrieve %s. %s might not be properly assigned."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(UFactionRecordDatabase),
			ARGUS_NAMEOF(m_factionRecordDatabase)
		);
		return nullptr;
	}

	return factionRecordDatabase->GetRecord(id);
}
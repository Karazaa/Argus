// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusStaticDatabase.h"

const UFactionRecord* UArgusStaticDatabase::GetUFactionRecord(uint32 id) const
{
	const UFactionRecordDatabase* factionRecordDatabase = m_factionRecordDatabase.LoadSynchronous();
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

const uint32 UArgusStaticDatabase::GetIdFromRecordSoftPtr(const TSoftObjectPtr<UFactionRecord>& UFactionRecord) const
{
	const UFactionRecordDatabase* factionRecordDatabase = m_factionRecordDatabase.LoadSynchronous();
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
		return 0u;
	}

	return factionRecordDatabase->GetIdFromRecordSoftPtr(UFactionRecord);
}
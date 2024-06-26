// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FactionRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

const UFactionRecord* UFactionRecordDatabase::GetRecord(uint32 id) const
{
	if (static_cast<uint32>(m_factionRecords.Num()) <= id)
	{
		UE_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not find %s %d in %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(id),
			id,
			ARGUS_NAMEOF(UFactionRecordDatabase)
		);

		return nullptr;
	}

	if (id == 0u)
	{
		return nullptr;
	}

	UFactionRecord* record = m_factionRecords[id].LoadSynchronous();
	if (record)
	{
		record->m_id = id;
	}

	return record;
}

const uint32 UFactionRecordDatabase::GetIdFromRecordSoftPtr(const TSoftObjectPtr<UFactionRecord>& UFactionRecord) const
{
	for (int i = 0; i < m_factionRecords.Num(); ++i)
	{
		if (m_factionRecords[i] == UFactionRecord)
		{
			return i;
		}
	}

	return 0u;
}
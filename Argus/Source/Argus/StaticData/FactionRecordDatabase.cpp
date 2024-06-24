// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FactionRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

const UFactionRecord* UFactionRecordDatabase::GetRecord(uint8 id) const
{
	if (!m_factionRecords.Contains(id))
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

	return m_factionRecords[id].LoadSynchronous();
}
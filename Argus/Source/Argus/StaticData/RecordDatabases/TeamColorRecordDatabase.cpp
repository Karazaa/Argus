// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "RecordDatabases/TeamColorRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

const UTeamColorRecord* UTeamColorRecordDatabase::GetRecord(uint32 id)
{
	if (static_cast<uint32>(m_UTeamColorRecordsPersistent.Num()) <= id)
	{
		ResizePersistentObjectPointerArray();
		
		if (static_cast<uint32>(m_UTeamColorRecordsPersistent.Num()) <= id)
		{
			UE_LOG
			(
				ArgusStaticDataLog, Error,
				TEXT("[%s] Could not find %s %d in %s."),
				ARGUS_FUNCNAME,
				ARGUS_NAMEOF(id),
				id,
				ARGUS_NAMEOF(UTeamColorRecordDatabase)
			);

			return nullptr;
		}
	}

	if (id == 0u)
	{
		return nullptr;
	}

	if (!m_UTeamColorRecordsPersistent[id])
	{
		m_UTeamColorRecordsPersistent[id] = m_UTeamColorRecords[id].LoadSynchronous();
	}

	if (m_UTeamColorRecordsPersistent[id])
	{
		m_UTeamColorRecordsPersistent[id]->m_id = id;
	}

	return m_UTeamColorRecordsPersistent[id];
}

const uint32 UTeamColorRecordDatabase::GetIdFromRecordSoftPtr(const TSoftObjectPtr<UTeamColorRecord>& UTeamColorRecord) const
{
	for (int i = 0; i < m_UTeamColorRecords.Num(); ++i)
	{
		if (m_UTeamColorRecords[i] == UTeamColorRecord)
		{
			return i;
		}
	}

	return 0u;
}

void UTeamColorRecordDatabase::ResizePersistentObjectPointerArray()
{
	m_UTeamColorRecordsPersistent.SetNumZeroed(m_UTeamColorRecords.Num());
}

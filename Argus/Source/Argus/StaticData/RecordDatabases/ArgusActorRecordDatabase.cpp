// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "RecordDatabases/ArgusActorRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

const UArgusActorRecord* UArgusActorRecordDatabase::GetRecord(uint32 id)
{
	if (static_cast<uint32>(m_UArgusActorRecordsPersistent.Num()) <= id)
	{
		ResizePersistentObjectPointerArray();
		
		if (static_cast<uint32>(m_UArgusActorRecordsPersistent.Num()) <= id)
		{
			ARGUS_LOG
			(
				ArgusStaticDataLog, Error,
				TEXT("[%s] Could not find %s %d in %s."),
				ARGUS_FUNCNAME,
				ARGUS_NAMEOF(id),
				id,
				ARGUS_NAMEOF(UArgusActorRecordDatabase)
			);

			return nullptr;
		}
	}

	if (id == 0u)
	{
		return nullptr;
	}

	if (!m_UArgusActorRecordsPersistent[id])
	{
		m_UArgusActorRecordsPersistent[id] = m_UArgusActorRecords[id].LoadSynchronous();
	}

	if (m_UArgusActorRecordsPersistent[id])
	{
		m_UArgusActorRecordsPersistent[id]->m_id = id;
	}

	return m_UArgusActorRecordsPersistent[id];
}

const uint32 UArgusActorRecordDatabase::GetIdFromRecordSoftPtr(const TSoftObjectPtr<UArgusActorRecord>& UArgusActorRecord) const
{
	for (int i = 0; i < m_UArgusActorRecords.Num(); ++i)
	{
		if (m_UArgusActorRecords[i] == UArgusActorRecord)
		{
			return i;
		}
	}

	return 0u;
}

void UArgusActorRecordDatabase::ResizePersistentObjectPointerArray()
{
	m_UArgusActorRecordsPersistent.SetNumZeroed(m_UArgusActorRecords.Num());
}

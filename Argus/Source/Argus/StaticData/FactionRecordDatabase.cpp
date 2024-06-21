// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FactionRecordDatabase.h"

const UFactionRecord* UFactionRecordDatabase::GetRecord(uint8 id) const
{
	if (!m_factionRecords.Contains(id))
	{
		return nullptr;
	}

	return m_factionRecords[id].LoadSynchronous();
}
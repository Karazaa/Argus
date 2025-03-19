// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ArgusStaticDatabase.h"

#pragma region UAbilityRecord
const UAbilityRecord* UArgusStaticDatabase::GetUAbilityRecord(uint32 id)
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	if (!m_UAbilityRecordDatabasePersistent)
	{
		m_UAbilityRecordDatabasePersistent = m_UAbilityRecordDatabase.LoadSynchronous();
		if (!m_UAbilityRecordDatabasePersistent)
		{
			ARGUS_LOG(ArgusStaticDataLog, Error, TEXT("[%s] Could not find %s reference. Need to set reference in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_UAbilityRecordDatabase), ARGUS_NAMEOF(UArgusStaticDatabase));
			return nullptr;
		}

		m_UAbilityRecordDatabasePersistent->ResizePersistentObjectPointerArray();
	}

	if (!m_UAbilityRecordDatabasePersistent)
	{
		ARGUS_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not retrieve %s. %s might not be properly assigned."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_UAbilityRecordDatabasePersistent),
			ARGUS_NAMEOF(m_UAbilityRecordDatabase)
		);
		return nullptr;
	}

	return m_UAbilityRecordDatabasePersistent->GetRecord(id);
}
#pragma endregion
#pragma region UArgusActorRecord
const UArgusActorRecord* UArgusStaticDatabase::GetUArgusActorRecord(uint32 id)
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	if (!m_UArgusActorRecordDatabasePersistent)
	{
		m_UArgusActorRecordDatabasePersistent = m_UArgusActorRecordDatabase.LoadSynchronous();
		if (!m_UArgusActorRecordDatabasePersistent)
		{
			ARGUS_LOG(ArgusStaticDataLog, Error, TEXT("[%s] Could not find %s reference. Need to set reference in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_UArgusActorRecordDatabase), ARGUS_NAMEOF(UArgusStaticDatabase));
			return nullptr;
		}

		m_UArgusActorRecordDatabasePersistent->ResizePersistentObjectPointerArray();
	}

	if (!m_UArgusActorRecordDatabasePersistent)
	{
		ARGUS_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not retrieve %s. %s might not be properly assigned."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_UArgusActorRecordDatabasePersistent),
			ARGUS_NAMEOF(m_UArgusActorRecordDatabase)
		);
		return nullptr;
	}

	return m_UArgusActorRecordDatabasePersistent->GetRecord(id);
}
#pragma endregion
#pragma region UFactionRecord
const UFactionRecord* UArgusStaticDatabase::GetUFactionRecord(uint32 id)
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	if (!m_UFactionRecordDatabasePersistent)
	{
		m_UFactionRecordDatabasePersistent = m_UFactionRecordDatabase.LoadSynchronous();
		if (!m_UFactionRecordDatabasePersistent)
		{
			ARGUS_LOG(ArgusStaticDataLog, Error, TEXT("[%s] Could not find %s reference. Need to set reference in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_UFactionRecordDatabase), ARGUS_NAMEOF(UArgusStaticDatabase));
			return nullptr;
		}

		m_UFactionRecordDatabasePersistent->ResizePersistentObjectPointerArray();
	}

	if (!m_UFactionRecordDatabasePersistent)
	{
		ARGUS_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not retrieve %s. %s might not be properly assigned."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_UFactionRecordDatabasePersistent),
			ARGUS_NAMEOF(m_UFactionRecordDatabase)
		);
		return nullptr;
	}

	return m_UFactionRecordDatabasePersistent->GetRecord(id);
}
#pragma endregion
#pragma region UPlacedArgusActorTeamInfoRecord
const UPlacedArgusActorTeamInfoRecord* UArgusStaticDatabase::GetUPlacedArgusActorTeamInfoRecord(uint32 id)
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	if (!m_UPlacedArgusActorTeamInfoRecordDatabasePersistent)
	{
		m_UPlacedArgusActorTeamInfoRecordDatabasePersistent = m_UPlacedArgusActorTeamInfoRecordDatabase.LoadSynchronous();
		if (!m_UPlacedArgusActorTeamInfoRecordDatabasePersistent)
		{
			ARGUS_LOG(ArgusStaticDataLog, Error, TEXT("[%s] Could not find %s reference. Need to set reference in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_UPlacedArgusActorTeamInfoRecordDatabase), ARGUS_NAMEOF(UArgusStaticDatabase));
			return nullptr;
		}

		m_UPlacedArgusActorTeamInfoRecordDatabasePersistent->ResizePersistentObjectPointerArray();
	}

	if (!m_UPlacedArgusActorTeamInfoRecordDatabasePersistent)
	{
		ARGUS_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not retrieve %s. %s might not be properly assigned."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_UPlacedArgusActorTeamInfoRecordDatabasePersistent),
			ARGUS_NAMEOF(m_UPlacedArgusActorTeamInfoRecordDatabase)
		);
		return nullptr;
	}

	return m_UPlacedArgusActorTeamInfoRecordDatabasePersistent->GetRecord(id);
}
#pragma endregion
#pragma region UTeamColorRecord
const UTeamColorRecord* UArgusStaticDatabase::GetUTeamColorRecord(uint32 id)
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	LazyLoadUTeamColorRecordDatabase();

	if (!m_UTeamColorRecordDatabasePersistent)
	{
		return nullptr;
	}

	return m_UTeamColorRecordDatabasePersistent->GetRecord(id);
}

#if WITH_EDITOR
const uint32 UArgusStaticDatabase::AddUTeamColorRecordToDatabase(UTeamColorRecord* record)
{
	LazyLoadUTeamColorRecordDatabase();

	if (!m_UTeamColorRecordDatabasePersistent)
	{
		return 0u;
	}

	m_UTeamColorRecordDatabasePersistent->AddUTeamColorRecordToDatabase(record);
	
	return record->m_id;
}
#endif

void UArgusStaticDatabase::LazyLoadUTeamColorRecordDatabase()
{
	if (!m_UTeamColorRecordDatabasePersistent)
	{
		m_UTeamColorRecordDatabasePersistent = m_UTeamColorRecordDatabase.LoadSynchronous();
		if (!m_UTeamColorRecordDatabasePersistent)
		{
			ARGUS_LOG(ArgusStaticDataLog, Error, TEXT("[%s] Could not find %s reference. Need to set reference in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_UTeamColorRecordDatabase), ARGUS_NAMEOF(UArgusStaticDatabase));
			return;
		}

		m_UTeamColorRecordDatabasePersistent->ResizePersistentObjectPointerArray();
	}

	if (!m_UTeamColorRecordDatabasePersistent)
	{
		ARGUS_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not retrieve %s. %s might not be properly assigned."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_UTeamColorRecordDatabasePersistent),
			ARGUS_NAMEOF(m_UTeamColorRecordDatabase)
		);
		return;
	}
}
#pragma endregion

// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "RecordDatabases/FactionRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

#if WITH_EDITOR
#include "Subsystems/EditorAssetSubsystem.h"
#endif

const UFactionRecord* UFactionRecordDatabase::GetRecord(uint32 id)
{
	if (static_cast<uint32>(m_UFactionRecordsPersistent.Num()) <= id)
	{
		ResizePersistentObjectPointerArray();
		
		if (static_cast<uint32>(m_UFactionRecordsPersistent.Num()) <= id)
		{
			ARGUS_LOG
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
	}

	if (id == 0u)
	{
		return nullptr;
	}

	if (!m_UFactionRecordsPersistent[id])
	{
		m_UFactionRecordsPersistent[id] = m_UFactionRecords[id].LoadSynchronous();
	}

	if (m_UFactionRecordsPersistent[id])
	{
		m_UFactionRecordsPersistent[id]->m_id = id;
	}

	return m_UFactionRecordsPersistent[id];
}

const uint32 UFactionRecordDatabase::GetIdFromRecordSoftPtr(const TSoftObjectPtr<UFactionRecord>& UFactionRecord) const
{
	for (int i = 0; i < m_UFactionRecords.Num(); ++i)
	{
		if (m_UFactionRecords[i] == UFactionRecord)
		{
			return i;
		}
	}

	return 0u;
}

void UFactionRecordDatabase::ResizePersistentObjectPointerArray()
{
	m_UFactionRecordsPersistent.SetNumZeroed(m_UFactionRecords.Num());
}

#if WITH_EDITOR
void UFactionRecordDatabase::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	if (propertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	const FString propertyName = propertyChangedEvent.GetPropertyName().ToString();
	const FString recordPropertyName = ARGUS_NAMEOF(m_UFactionRecords);
	if (!propertyName.Equals(recordPropertyName))
	{
		return;
	}

	const int32 arrayIndex = propertyChangedEvent.GetArrayIndex(propertyName);
	UFactionRecord* modifiedUFactionRecord = m_UFactionRecords[arrayIndex].LoadSynchronous();
	if (!modifiedUFactionRecord)
	{
		return;
	}

	modifiedUFactionRecord->m_id = arrayIndex;

	if (!GEditor)
	{
		return;
	}

	UEditorAssetSubsystem* editorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	if (!editorAssetSubsystem)
	{
		return;
	}

	editorAssetSubsystem->SaveLoadedAsset(modifiedUFactionRecord, false);
	editorAssetSubsystem->SaveLoadedAsset(this, false);
}
#endif //WITH_EDITOR

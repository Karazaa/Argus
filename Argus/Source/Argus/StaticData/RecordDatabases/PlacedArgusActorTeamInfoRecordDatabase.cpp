// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "RecordDatabases/PlacedArgusActorTeamInfoRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
#endif

const UPlacedArgusActorTeamInfoRecord* UPlacedArgusActorTeamInfoRecordDatabase::GetRecord(uint32 id)
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	if (static_cast<uint32>(m_UPlacedArgusActorTeamInfoRecordsPersistent.Num()) <= id)
	{
		ResizePersistentObjectPointerArray();
		
		if (static_cast<uint32>(m_UPlacedArgusActorTeamInfoRecordsPersistent.Num()) <= id)
		{
			ARGUS_LOG
			(
				ArgusStaticDataLog, Error,
				TEXT("[%s] Could not find %s %d in %s."),
				ARGUS_FUNCNAME,
				ARGUS_NAMEOF(id),
				id,
				ARGUS_NAMEOF(UPlacedArgusActorTeamInfoRecordDatabase)
			);

			return nullptr;
		}
	}

	if (id == 0u)
	{
		return nullptr;
	}

	if (!m_UPlacedArgusActorTeamInfoRecordsPersistent[id])
	{
		m_UPlacedArgusActorTeamInfoRecordsPersistent[id] = m_UPlacedArgusActorTeamInfoRecords[id].LoadSynchronous();
	}

	if (m_UPlacedArgusActorTeamInfoRecordsPersistent[id])
	{
		m_UPlacedArgusActorTeamInfoRecordsPersistent[id]->m_id = id;
	}

	return m_UPlacedArgusActorTeamInfoRecordsPersistent[id];
}

void UPlacedArgusActorTeamInfoRecordDatabase::ResizePersistentObjectPointerArray()
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	m_UPlacedArgusActorTeamInfoRecordsPersistent.SetNumZeroed(m_UPlacedArgusActorTeamInfoRecords.Num());
}

#if WITH_EDITOR
void UPlacedArgusActorTeamInfoRecordDatabase::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	if (propertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	const FString propertyName = propertyChangedEvent.GetPropertyName().ToString();
	const FString recordPropertyName = ARGUS_NAMEOF(m_UPlacedArgusActorTeamInfoRecords);
	if (!propertyName.Equals(recordPropertyName))
	{
		return;
	}

	const int32 arrayIndex = propertyChangedEvent.GetArrayIndex(propertyName);
	UPlacedArgusActorTeamInfoRecord* modifiedUPlacedArgusActorTeamInfoRecord = m_UPlacedArgusActorTeamInfoRecords[arrayIndex].LoadSynchronous();
	if (!modifiedUPlacedArgusActorTeamInfoRecord)
	{
		return;
	}

	modifiedUPlacedArgusActorTeamInfoRecord->m_id = arrayIndex;

	if (!GEditor)
	{
		return;
	}

	UEditorAssetSubsystem* editorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	if (!editorAssetSubsystem)
	{
		return;
	}

	editorAssetSubsystem->SaveLoadedAsset(modifiedUPlacedArgusActorTeamInfoRecord, false);
	editorAssetSubsystem->SaveLoadedAsset(this, false);
}

void UPlacedArgusActorTeamInfoRecordDatabase::AddUPlacedArgusActorTeamInfoRecordToDatabase(UPlacedArgusActorTeamInfoRecord* record)
{
	const int32 arrayIndex = m_UPlacedArgusActorTeamInfoRecords.Num();
	m_UPlacedArgusActorTeamInfoRecords.Add(TSoftObjectPtr(record));

	record->m_id = arrayIndex;

	if (!GEditor)
	{
		return;
	}

	UEditorAssetSubsystem* editorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	if (!editorAssetSubsystem)
	{
		return;
	}

	editorAssetSubsystem->SaveLoadedAsset(this, false);
}
#endif //WITH_EDITOR

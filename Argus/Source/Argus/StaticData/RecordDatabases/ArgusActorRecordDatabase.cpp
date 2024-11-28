// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "RecordDatabases/ArgusActorRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

#if WITH_EDITOR
#include "Subsystems/EditorAssetSubsystem.h"
#endif

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

void UArgusActorRecordDatabase::ResizePersistentObjectPointerArray()
{
	m_UArgusActorRecordsPersistent.SetNumZeroed(m_UArgusActorRecords.Num());
}

#if WITH_EDITOR
void UArgusActorRecordDatabase::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	if (propertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	const FString propertyName = propertyChangedEvent.GetPropertyName().ToString();
	const FString recordPropertyName = ARGUS_NAMEOF(m_UArgusActorRecords);
	if (!propertyName.Equals(recordPropertyName))
	{
		return;
	}

	const int32 arrayIndex = propertyChangedEvent.GetArrayIndex(propertyName);
	UArgusActorRecord* modifiedUArgusActorRecord = m_UArgusActorRecords[arrayIndex].LoadSynchronous();
	if (!modifiedUArgusActorRecord)
	{
		return;
	}

	modifiedUArgusActorRecord->m_id = arrayIndex;

	if (!GEditor)
	{
		return;
	}

	UEditorAssetSubsystem* editorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	if (!editorAssetSubsystem)
	{
		return;
	}

	editorAssetSubsystem->SaveLoadedAsset(modifiedUArgusActorRecord, false);
	editorAssetSubsystem->SaveLoadedAsset(this, false);
}
#endif //WITH_EDITOR

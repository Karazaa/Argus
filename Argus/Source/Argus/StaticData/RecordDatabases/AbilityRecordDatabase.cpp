// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "RecordDatabases/AbilityRecordDatabase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
#endif

const UAbilityRecord* UAbilityRecordDatabase::GetRecord(uint32 id)
{
	if (static_cast<uint32>(m_UAbilityRecordsPersistent.Num()) <= id)
	{
		ResizePersistentObjectPointerArray();
		
		if (static_cast<uint32>(m_UAbilityRecordsPersistent.Num()) <= id)
		{
			ARGUS_LOG
			(
				ArgusStaticDataLog, Error,
				TEXT("[%s] Could not find %s %d in %s."),
				ARGUS_FUNCNAME,
				ARGUS_NAMEOF(id),
				id,
				ARGUS_NAMEOF(UAbilityRecordDatabase)
			);

			return nullptr;
		}
	}

	if (id == 0u)
	{
		return nullptr;
	}

	if (!m_UAbilityRecordsPersistent[id])
	{
		m_UAbilityRecordsPersistent[id] = m_UAbilityRecords[id].LoadSynchronous();
	}

	if (m_UAbilityRecordsPersistent[id])
	{
		m_UAbilityRecordsPersistent[id]->m_id = id;
	}

	return m_UAbilityRecordsPersistent[id];
}

void UAbilityRecordDatabase::ResizePersistentObjectPointerArray()
{
	m_UAbilityRecordsPersistent.SetNumZeroed(m_UAbilityRecords.Num());
}

#if WITH_EDITOR
void UAbilityRecordDatabase::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	if (propertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	const FString propertyName = propertyChangedEvent.GetPropertyName().ToString();
	const FString recordPropertyName = ARGUS_NAMEOF(m_UAbilityRecords);
	if (!propertyName.Equals(recordPropertyName))
	{
		return;
	}

	const int32 arrayIndex = propertyChangedEvent.GetArrayIndex(propertyName);
	UAbilityRecord* modifiedUAbilityRecord = m_UAbilityRecords[arrayIndex].LoadSynchronous();
	if (!modifiedUAbilityRecord)
	{
		return;
	}

	modifiedUAbilityRecord->m_id = arrayIndex;

	if (!GEditor)
	{
		return;
	}

	UEditorAssetSubsystem* editorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	if (!editorAssetSubsystem)
	{
		return;
	}

	editorAssetSubsystem->SaveLoadedAsset(modifiedUAbilityRecord, false);
	editorAssetSubsystem->SaveLoadedAsset(this, false);
}
#endif //WITH_EDITOR

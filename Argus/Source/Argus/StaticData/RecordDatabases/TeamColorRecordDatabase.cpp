// Copayright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "RecordDatabases/TeamColorRecordDatabase.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"

#if WITH_EDITOR && !IS_PACKAGING_ARGUS
#include "ArgusStaticData.h"
#include "Editor.h"
#include "Misc/Paths.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UObject/ObjectSaveContext.h"
#include <filesystem>
#endif //WITH_EDITOR && !IS_PACKAGING_ARGUS

const UTeamColorRecord* UTeamColorRecordDatabase::GetRecord(uint32 id)
{
	ARGUS_TRACE(UTeamColorRecordDatabase::GetRecord)
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	bool resized = false;
	if (static_cast<uint32>(m_UTeamColorRecordsPersistent.Num()) <= id)
	{
		if (!ResizePersistentObjectPointerArrayToFitRecord(id))
		{
			return nullptr;
		}		
		resized = true;
	}

	if (id == 0u)
	{
		return nullptr;
	}

	if (resized || !m_UTeamColorRecordsPersistent[id])
	{
		m_UTeamColorRecordsPersistent[id] = m_UTeamColorRecords[id].LoadSynchronous();
	}

	if (m_UTeamColorRecordsPersistent[id])
	{
		m_UTeamColorRecordsPersistent[id]->m_id = id;
	}

	return m_UTeamColorRecordsPersistent[id];
}

const bool UTeamColorRecordDatabase::AsyncPreLoadRecord(uint32 id)
{
	ARGUS_TRACE(UTeamColorRecordDatabase::AsyncPreLoadRecord);
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	if (static_cast<uint32>(m_UTeamColorRecordsPersistent.Num()) <= id)
	{
		if (!ResizePersistentObjectPointerArrayToFitRecord(id))
		{
			return false;
		}
	}

	if (id == 0u)
	{
		return false;
	}

	if (m_UTeamColorRecordsPersistent[id])
	{
		return true;
	}

	AssetLoadingComponent* assetLoadingComponent = ArgusEntity::GetSingletonEntity().GetComponent<AssetLoadingComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(assetLoadingComponent, ArgusStaticDataLog);

	assetLoadingComponent->m_streamableManager.RequestAsyncLoad(m_UTeamColorRecords[id].ToSoftObjectPath(), FStreamableDelegate::CreateLambda
	(
		[this, id]()
		{
			if (static_cast<uint32>(m_UTeamColorRecordsPersistent.Num()) <= id || static_cast<uint32>(m_UTeamColorRecords.Num()) <= id)
			{
				return;
			}

			m_UTeamColorRecordsPersistent[id] = m_UTeamColorRecords[id].Get();\
			if (m_UTeamColorRecordsPersistent[id])
			{
				m_UTeamColorRecordsPersistent[id]->OnAsyncLoaded();
			}
		})
	);

	return true;
}

bool UTeamColorRecordDatabase::ResizePersistentObjectPointerArrayToFitRecord(uint32 id)
{
	ARGUS_MEMORY_TRACE(ArgusStaticData);

	m_UTeamColorRecordsPersistent.SetNumZeroed(m_UTeamColorRecords.Num());
	if (static_cast<uint32>(m_UTeamColorRecordsPersistent.Num()) <= id)
	{
		ARGUS_LOG
		(
			ArgusStaticDataLog, Error,
			TEXT("[%s] Could not find %s %d in %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(id),
			id,
			ARGUS_NAMEOF(UTeamColorRecordDatabase)
		);

		return false;
	}

	return true;
}

#if WITH_EDITOR && !IS_PACKAGING_ARGUS
void UTeamColorRecordDatabase::PreSave(FObjectPreSaveContext saveContext)
{
	FString fullPath = FPaths::ConvertRelativePathToFull(saveContext.GetTargetFilename());
	if (!std::filesystem::exists(TCHAR_TO_UTF8(*fullPath)))
	{
		TArray<FReferencerInformation> internalReferencers;
		TArray<FReferencerInformation> externalReferencers;
		RetrieveReferencers(&internalReferencers, &externalReferencers);

		if (internalReferencers.IsEmpty() && externalReferencers.IsEmpty())
		{
			ArgusStaticData::RegisterNewUTeamColorRecordDatabase(this);
		}
	}

	Super::PreSave(saveContext);
}

void UTeamColorRecordDatabase::PostEditChangeProperty(FPropertyChangedEvent& propertyChangedEvent)
{
	if (propertyChangedEvent.ChangeType != EPropertyChangeType::ValueSet)
	{
		return;
	}

	const FString propertyName = propertyChangedEvent.GetPropertyName().ToString();
	const FString recordPropertyName = ARGUS_NAMEOF(m_UTeamColorRecords);
	if (!propertyName.Equals(recordPropertyName))
	{
		return;
	}

	const int32 arrayIndex = propertyChangedEvent.GetArrayIndex(propertyName);
	UTeamColorRecord* modifiedUTeamColorRecord = m_UTeamColorRecords[arrayIndex].LoadSynchronous();
	if (!modifiedUTeamColorRecord)
	{
		return;
	}

	modifiedUTeamColorRecord->m_id = arrayIndex;

	if (!GEditor)
	{
		return;
	}

	UEditorAssetSubsystem* editorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
	if (!editorAssetSubsystem)
	{
		return;
	}

	editorAssetSubsystem->SaveLoadedAsset(modifiedUTeamColorRecord, false);
	editorAssetSubsystem->SaveLoadedAsset(this, false);
}

void UTeamColorRecordDatabase::AddUTeamColorRecordToDatabase(UTeamColorRecord* record)
{
	const int32 arrayIndex = m_UTeamColorRecords.Num();
	m_UTeamColorRecords.Add(TSoftObjectPtr(record));

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
#endif //WITH_EDITOR && !IS_PACKAGING_ARGUS

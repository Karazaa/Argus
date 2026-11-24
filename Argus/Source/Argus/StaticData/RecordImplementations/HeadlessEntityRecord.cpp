// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/HeadlessEntityRecord.h"
#include "ArgusStaticData.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Engine/World.h"
#include "RecordDefinitions/WorldCellRecord.h"
#include "Systems/WorldCellSystems.h"
#endif // WITH_EDITOR

FString UHeadlessEntityRecord::GetPackagePath(const FString& assetName)
{
	return FString::Printf(TEXT("/Game/StaticData/HeadlessEntityRecords/%s"), *assetName);
}

void UHeadlessEntityRecord::OnAsyncLoaded() const
{
#if WITH_EDITOR
	if (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != nullptr)
	{
		m_entityTemplate.AsyncPreLoadAndStorePtr();
	}
	else
	{
		m_entityTemplate.LoadAndStorePtr();
	}
#else
	m_entityTemplate.AsyncPreLoadAndStorePtr();
#endif // WITH_EDITOR
}

void UHeadlessEntityRecord::ResetSoftPtrLoadStores()
{
	m_entityTemplate.ResetHardPtr();
}

#if WITH_EDITOR
bool UHeadlessEntityRecord::UpdateRecord(UWorld* world, const FVector& initialLocation, const FSoftObjectLoadStore_UArgusEntityTemplate& entityTemplate)
{
	ARGUS_RETURN_ON_NULL_BOOL(world, ArgusStaticDataLog);

	UWorldCellRecord* newWorldCellRecord = WorldCellSystems::GetWorldCellRecordForLocation(world, initialLocation);
	ARGUS_RETURN_ON_NULL_BOOL(newWorldCellRecord, ArgusStaticDataLog);
	UWorldCellRecord* existingWorldCell = nullptr;
	bool needsAddWorldCell = true;

	if (m_entityTemplate)
	{
		existingWorldCell = WorldCellSystems::GetWorldCellRecordForLocation(world, m_initialLocation);
		ARGUS_RETURN_ON_NULL_BOOL(existingWorldCell, ArgusStaticDataLog);
		needsAddWorldCell = newWorldCellRecord != existingWorldCell;
	}

	m_initialLocation = initialLocation;
	m_entityTemplate = entityTemplate;

	Modify(true);
	bool saveResult = SaveRecord();

	if (saveResult && needsAddWorldCell)
	{
		FUHeadlessEntityRecordReference& headlessEntityRecordReference = newWorldCellRecord->m_headlessEntities.AddDefaulted_GetRef();
		headlessEntityRecordReference.CacheRecordAsReference(this);

		newWorldCellRecord->Modify(true);
		saveResult = newWorldCellRecord->SaveRecord();

		if (existingWorldCell)
		{
			const int32 numRemoved = existingWorldCell->m_headlessEntities.RemoveAll([id = m_id](const FUHeadlessEntityRecordReference& entry)
			{
				return entry.GetId() == id;
			});

			if (numRemoved > 0)
			{
				existingWorldCell->Modify(true);
				saveResult = existingWorldCell->SaveRecord();
			}
		}
	}

	return saveResult;
}

bool UHeadlessEntityRecord::UnregisterSelfFromWorldCell(UWorld* world) const
{
	UWorldCellRecord* existingWorldCell = WorldCellSystems::GetWorldCellRecordForLocation(world, m_initialLocation);
	ARGUS_RETURN_ON_NULL_BOOL(existingWorldCell, ArgusStaticDataLog);

	const int32 numRemoved = existingWorldCell->m_headlessEntities.RemoveAll([id = m_id](const FUHeadlessEntityRecordReference& entry)
	{
		return entry.GetId() == id;
	});

	bool saveResult = true;
	if (numRemoved > 0)
	{
		existingWorldCell->Modify(true);
		saveResult = existingWorldCell->SaveRecord();
	}

	return saveResult;
}
#endif // WITH_EDITOR
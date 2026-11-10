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

	bool needsAddWorldCell = true;
	if (m_entityTemplate)
	{
		// TODO JAMES: Remove world cell record instance of headless entity record.
		needsAddWorldCell = false;
	}

	m_initialLocation = initialLocation;
	m_entityTemplate = entityTemplate;

	Modify(true);
	bool saveResult = SaveRecord();

	if (saveResult && needsAddWorldCell)
	{
		TSoftObjectPtr<UWorld> worldSoftPtr = TSoftObjectPtr<UWorld>(world);

		// TODO JAMES: We eventually need to get the indices of the world cell record from initialLocation. We don't have a path to do that at edit time however.
		UWorldCellRecord* worldCellRecord = const_cast<UWorldCellRecord*>(WorldCellSystems::GetWorldCellRecordForIndicies(worldSoftPtr, 0, 0));
		ARGUS_RETURN_ON_NULL_BOOL(worldCellRecord, ArgusStaticDataLog);

		FUHeadlessEntityRecordReference& headlessEntityRecordReference = worldCellRecord->m_headlessEntities.AddDefaulted_GetRef();
		headlessEntityRecordReference.CacheRecordAsReference(this);

		worldCellRecord->Modify(true);
		saveResult = worldCellRecord->SaveRecord();
	}

	return saveResult;
}
#endif // WITH_EDITOR
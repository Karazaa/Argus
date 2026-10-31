// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/HeadlessEntityRecord.h"
#include "ArgusStaticData.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Engine/World.h"
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
bool UHeadlessEntityRecord::UpdateRecord(const FVector& initialLocation, const FSoftObjectLoadStore_UArgusEntityTemplate& entityTemplate)
{
	m_initialLocation = initialLocation;
	m_entityTemplate = entityTemplate;
	Modify(true);
	return SaveRecord();
}
#endif // WITH_EDITOR
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/HeadlessEntityRecord.h"
#include "ArgusStaticData.h"

FString UHeadlessEntityRecord::GetPackagePath(const FString& assetName)
{
	return FString::Printf(TEXT("/Game/StaticData/HeadlessEntityRecords/%s"), *assetName);
}

void UHeadlessEntityRecord::OnAsyncLoaded() const
{
	m_entityTemplate.AsyncPreLoadAndStorePtr();
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
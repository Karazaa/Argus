// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "HeadlessEntityComponent.h"
#include "ArgusStaticData.h"
#include "FileHelpers.h"
#include "RecordDefinitions/HeadlessEntityRecord.h"

UHeadlessEntityComponent::UHeadlessEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsEditorOnly = true;
}

#if WITH_EDITOR
void UHeadlessEntityComponent::OnHeadlessActorUpdated(const FTransform& actorTransform)
{
	if (!m_entityTemplate)
	{
		return;
	}

	const uint32 recordId = m_headlessEntityRecordReference.GetId();
	if (recordId == 0u)
	{
		CreateNewHeadlessEntityRecord();
	}
	else
	{
		UpdateExistingHeadlessEntityRecord(recordId);
	}
}

void UHeadlessEntityComponent::CreateNewHeadlessEntityRecord()
{
	const AActor* owningActor = GetOwner();
	ARGUS_RETURN_ON_NULL(owningActor, ArgusUnrealObjectsLog);

	ArgusStaticData::LoadStaticDatabaseSynchronous();

	const FString recordName = FString::Printf(TEXT("HeadlessEntity_%s"), *owningActor->GetActorNameOrLabel());
	UPackage* package = CreatePackage(*UHeadlessEntityRecord::GetPackagePath(recordName));
	ARGUS_RETURN_ON_NULL(package, ArgusUnrealObjectsLog);

	UHeadlessEntityRecord* headlessEntityRecord = NewObject<UHeadlessEntityRecord>(package, UHeadlessEntityRecord::StaticClass(), *recordName, RF_Public | RF_Standalone);
	ARGUS_RETURN_ON_NULL(headlessEntityRecord, ArgusUnrealObjectsLog);

	headlessEntityRecord->UpdateRecord(owningActor->GetActorLocation(), m_entityTemplate);
	headlessEntityRecord->SaveRecord();
	m_headlessEntityRecordReference.CacheRecordAsReference(headlessEntityRecord);

	Modify();
	TArray<UPackage*> packagesToSave;
	packagesToSave.Add(GetPackage());
	FEditorFileUtils::PromptForCheckoutAndSave(packagesToSave, true, false);
}

void UHeadlessEntityComponent::UpdateExistingHeadlessEntityRecord(uint32 headlessEntityId)
{
	const AActor* owningActor = GetOwner();
	ARGUS_RETURN_ON_NULL(owningActor, ArgusUnrealObjectsLog);

	ArgusStaticData::LoadStaticDatabaseSynchronous();

	UHeadlessEntityRecord* headlessEntityRecord = ArgusStaticData::GetMutableRecord<UHeadlessEntityRecord>(headlessEntityId);
	ARGUS_RETURN_ON_NULL(headlessEntityRecord, ArgusUnrealObjectsLog);

	headlessEntityRecord->UpdateRecord(owningActor->GetActorLocation(), m_entityTemplate);
	headlessEntityRecord->SaveRecord();
}
#endif // WITH_EDITOR
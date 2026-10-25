// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "HeadlessEntityComponent.h"
#include "ArgusStaticData.h"
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
		CreateNewHeadlessEntityRecord(recordId);
	}
	else
	{
		UpdateExistingHeadlessEntityRecord(recordId);
	}
}

void UHeadlessEntityComponent::CreateNewHeadlessEntityRecord(uint32 headlessEntityId)
{
	// TODO JAMES: Create new record.
}

void UHeadlessEntityComponent::UpdateExistingHeadlessEntityRecord(uint32 headlessEntityId)
{
	const AActor* owningActor = GetOwner();
	ARGUS_RETURN_ON_NULL(owningActor, ArgusUnrealObjectsLog);

	ArgusStaticData::LoadStaticDatabaseSynchronous();

	UHeadlessEntityRecord* headlessEntityRecord = ArgusStaticData::GetMutableRecord<UHeadlessEntityRecord>(headlessEntityId);
	ARGUS_RETURN_ON_NULL(headlessEntityRecord, ArgusUnrealObjectsLog);

	headlessEntityRecord->UpdateRecord(owningActor->GetActorLocation(), m_entityTemplate);
}
#endif // WITH_EDITOR
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "HeadlessEntityComponent.h"

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
		// TODO JAMES: Create new record.
	}
	else
	{
		// TODO JAMES: Update existing record.
	}
}
#endif // WITH_EDITOR
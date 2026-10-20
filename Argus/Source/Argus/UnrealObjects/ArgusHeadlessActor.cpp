// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusHeadlessActor.h"
#include "Components/HeadlessComponentInterface.h"

AArgusHeadlessActor::AArgusHeadlessActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;
}

#if WITH_EDITOR
void AArgusHeadlessActor::OnHeadlessActorUpdated() const
{
	const FTransform actorTransform = GetActorTransform();
	const TSet<UActorComponent*>& actorComponents = GetComponents();

	for (UActorComponent* actorComponent : actorComponents)
	{
		if (!actorComponent)
		{
			continue;
		}

		if (IHeadlessComponentInterface* headlessComponent = Cast<IHeadlessComponentInterface>(actorComponent))
		{
			headlessComponent->OnHeadlessActorUpdated(actorTransform);
		}
	}
}
#endif // WITH_EDITOR
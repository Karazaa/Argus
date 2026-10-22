// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusHeadlessActor.h"
#include "Components/HeadlessEntityComponent.h"

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

		if (UHeadlessEntityComponent* headlessComponent = Cast<UHeadlessEntityComponent>(actorComponent))
		{
			headlessComponent->OnHeadlessActorUpdated(actorTransform);
		}
	}
}
#endif // WITH_EDITOR
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusHeadlessActor.h"

#if WITH_EDITOR
#include "ArgusLogging.h"
#include "Components/HeadlessEntityComponent.h"
#include "Editor.h"
#endif // WITH_EDITOR

AArgusHeadlessActor::AArgusHeadlessActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;
}

#if WITH_EDITOR
void AArgusHeadlessActor::Destroyed()
{
	if (IsInValidContextForRecordEditing())
	{
		IterateHeadlessEntityComponents([](UHeadlessEntityComponent* headlessComponent)
		{
			ARGUS_RETURN_ON_NULL(headlessComponent, ArgusUnrealObjectsLog);
			headlessComponent->OnHeadlessActorDestroyed();
		});
	}

	Super::Destroyed();
}

bool AArgusHeadlessActor::IsInValidContextForRecordEditing() const
{
	// Don't do record modifications when headless or PIE. 
	if (IsRunningCommandlet() || FApp::IsGame())
	{
		return false;
	}

	// Has to be owned by an actual placed actor.
	if (IsTemplate() || !GetLevel() || HasAllFlags(RF_Transient))
	{
		return false;
	}

	// Has to be a valid editor world.
	const UWorld* world = GetWorld();
	if (!IsValid(world) || world->WorldType != EWorldType::Editor)
	{
		return false;
	}

	return true;
}

void AArgusHeadlessActor::OnHeadlessActorUpdated(bool initialUpdate) const
{
	if (!IsInValidContextForRecordEditing())
	{
		return;
	}

	const FTransform actorTransform = GetActorTransform();

	IterateHeadlessEntityComponents([&actorTransform, initialUpdate](UHeadlessEntityComponent* headlessComponent)
	{
		ARGUS_RETURN_ON_NULL(headlessComponent, ArgusUnrealObjectsLog);

		if (initialUpdate)
		{
			headlessComponent->OnHeadlessActorAdded();
		}
		headlessComponent->OnHeadlessActorUpdated(actorTransform);
	});
}

void AArgusHeadlessActor::IterateHeadlessEntityComponents(const TFunctionRef<void(UHeadlessEntityComponent*)>& function) const
{
	const TSet<UActorComponent*>& actorComponents = GetComponents();

	for (UActorComponent* actorComponent : actorComponents)
	{
		if (!actorComponent)
		{
			continue;
		}

		if (UHeadlessEntityComponent* headlessComponent = Cast<UHeadlessEntityComponent>(actorComponent))
		{
			function(headlessComponent);
		}
	}
}
#endif // WITH_EDITOR
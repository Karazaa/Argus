// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "HeadlessTriggerComponent.h"

UHeadlessTriggerComponent::UHeadlessTriggerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsEditorOnly = true;
}

#if WITH_EDITOR
void UHeadlessTriggerComponent::OnHeadlessActorUpdated(const FTransform& actorTransform)
{

}
#endif // WITH_EDITOR
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Components/ActorComponent.h"
#include "Components/HeadlessComponentInterface.h"
#include "SoftPtrLoadStore.h"
#include "HeadlessTriggerComponent.generated.h"

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent, DisableNativeTick))
class UHeadlessTriggerComponent : public UActorComponent, public IHeadlessComponentInterface
{
	GENERATED_BODY()

public:
	UHeadlessTriggerComponent();

#if WITH_EDITOR
	virtual void OnHeadlessActorUpdated(const FTransform& actorTransform) override;
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FSoftObjectLoadStore_UArgusEntityTemplate m_triggerEntityTemplate;
#endif // WITH_EDITORONLY_DATA
};
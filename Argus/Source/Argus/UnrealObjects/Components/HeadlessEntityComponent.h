// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Components/ActorComponent.h"
#include "RecordReferences/HeadlessEntityRecordReference.h"
#include "SoftPtrLoadStore.h"
#include "HeadlessEntityComponent.generated.h"

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent, DisableNativeTick))
class UHeadlessEntityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeadlessEntityComponent();

#if WITH_EDITOR
	virtual void OnHeadlessActorUpdated(const FTransform& actorTransform);
protected:
	virtual void CreateNewHeadlessEntityRecord(uint32 headlessEntityRecordId);
	virtual void UpdateExistingHeadlessEntityRecord(uint32 headlessEntityRecordId);
#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere)
	FSoftObjectLoadStore_UArgusEntityTemplate m_entityTemplate;

	UPROPERTY(VisibleAnywhere)
	FUHeadlessEntityRecordReference m_headlessEntityRecordReference;
#endif // WITH_EDITORONLY_DATA
};
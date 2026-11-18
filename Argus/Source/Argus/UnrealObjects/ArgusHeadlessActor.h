// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "GameFramework/Actor.h"
#include "ArgusHeadlessActor.generated.h"

#if WITH_EDITOR
class UHeadlessEntityComponent;
#endif // WITH_EDITOR

UCLASS(meta = (DisableNativeTick))
class ARGUS_API AArgusHeadlessActor : public AActor
{
	GENERATED_BODY()

public:
	AArgusHeadlessActor();

#if WITH_EDITOR
	virtual void Destroyed() override;

	bool IsInValidContextForRecordEditing() const;
	void OnHeadlessActorUpdated(bool initialUpdate) const;

private:
	void IterateHeadlessEntityComponents(const TFunctionRef<void(UHeadlessEntityComponent*)>& function) const;
#endif // WITH_EDITOR
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "GameFramework/Actor.h"
#include "ArgusHeadlessActor.generated.h"

UCLASS(meta = (DisableNativeTick))
class AArgusHeadlessActor : public AActor
{
	GENERATED_BODY()

public:
	AArgusHeadlessActor();

#if WITH_EDITOR
	void OnHeadlessActorUpdated() const;
#endif // WITH_EDITOR
};
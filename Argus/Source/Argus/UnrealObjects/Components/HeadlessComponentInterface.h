// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HeadlessComponentInterface.generated.h"

UINTERFACE()
class UHeadlessComponentInterface : public UInterface
{
	GENERATED_BODY()
};

class IHeadlessComponentInterface
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void OnHeadlessActorUpdated(const FTransform& actorTransform) = 0;
#endif // WITH_EDITOR
};
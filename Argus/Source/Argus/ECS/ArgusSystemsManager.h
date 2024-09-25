// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class UWorld;

class ArgusSystemsManager
{
private:
	static const uint16 s_singletonEntityId;

public:
	static void RunSystems(TWeakObjectPtr<UWorld> worldPointer, float deltaTime);
	static void UpdateSingletonComponents(bool didMovementUpdateThisFrame);
};
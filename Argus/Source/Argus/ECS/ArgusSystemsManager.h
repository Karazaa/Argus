// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class UWorld;

class ArgusSystemsManager
{
public:
	static const uint16 s_singletonEntityId;
	static void RunSystems(UWorld* worldPointer, float deltaTime);
	static void UpdateSingletonComponents(bool didMovementUpdateThisFrame);
};
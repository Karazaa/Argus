// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class UWorld;

class ArgusSystemsManager
{
public:
	static const uint16 s_singletonEntityId;
	static void RunSystems(UWorld* worldPointer, float deltaTime);

private:
	static void PopulateSingletonComponents();
	static void UpdateSingletonComponents(bool didMovementUpdateThisFrame);
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class UWorld;

class ArgusSystemsManager
{
public:
	static void RunSystems(TWeakObjectPtr<UWorld> worldPointer, float deltaTime);
};
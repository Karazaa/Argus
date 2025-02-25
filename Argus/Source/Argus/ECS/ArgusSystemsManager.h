// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class UWorld;

class ArgusSystemsManager
{
public:
	static void Initialize(UWorld* worldPointer);
	static void RunSystems(UWorld* worldPointer, float deltaTime);

private:
	static void PopulateSingletonComponents(UWorld* worldPointer);
	static void PopulateTeamComponents();
	static void UpdateSingletonComponents(bool didEntityPositionChangeThisFrame);
};
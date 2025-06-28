// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ResourceSet.h"
#include "CoreMinimal.h"

class UWorld;

class ArgusSystemsManager
{
public:
	static void Initialize(UWorld* worldPointer, const FResourceSet& initialTeamResourceSet);
	static void OnStartPlay(UWorld* worldPointer);
	static void RunSystems(UWorld* worldPointer, float deltaTime);

private:
	static void PopulateSingletonComponents(UWorld* worldPointer);
	static void SetInitialSingletonState(UWorld* worldPointer);
	static void PopulateTeamComponents(const FResourceSet& initialTeamResourceSet);
	static void UpdateSingletonComponents(bool didEntityPositionChangeThisFrame);
};
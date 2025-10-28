// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/ResourceSet.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "CoreMinimal.h"

class UArgusEntityTemplate;
class UWorld;

class ArgusSystemsManager
{
public:
	static void Initialize(UWorld* worldPointer, const FResourceSet& initialTeamResourceSet, const UArgusEntityTemplate* singletonTemplate);
	static void OnStartPlay(UWorld* worldPointer, ETeam activePlayerTeam);
	static void RunSystems(UWorld* worldPointer, float deltaTime);
	static void RunPostThreadSystems();

private:
	static void PopulateSingletonComponents(UWorld* worldPointer, const UArgusEntityTemplate* singletonTemplate);
	static void SetInitialSingletonState(UWorld* worldPointer, ETeam activePlayerTeam);
	static void PopulateTeamComponents(const FResourceSet& initialTeamResourceSet);
};
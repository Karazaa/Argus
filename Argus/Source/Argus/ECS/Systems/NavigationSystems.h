// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class NavigationSystems
{
public:
	static void RunSystems(TWeakObjectPtr<UWorld> worldPointer, float deltaTime);
	static void ProcessNavigationTaskCommands(TWeakObjectPtr<UWorld> worldPointer, ArgusEntity sourceEntity, NavigationComponent* sourceNavigationComponent);
	static void NavigateFromEntityToLocation(TWeakObjectPtr<UWorld> worldPointer, ArgusEntity sourceEntity, NavigationComponent* sourceNavigationComponent, FVector targetLocation);

private:
	static bool IsWorldPointerValidCheck(TWeakObjectPtr<UWorld> worldPointer);
	static bool IsSourceEntityValidCheck(ArgusEntity sourceEntity);
	static bool IsSourceNavigationComponentValidCheck(NavigationComponent* sourceNavigationComponent);
};
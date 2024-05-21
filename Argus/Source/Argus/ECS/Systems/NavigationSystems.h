// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class NavigationSystems
{
public:
	static void RunSystems(TWeakObjectPtr<UWorld> worldPointer);

	struct NavigationSystemsComponentArgs
	{
		TaskComponent* taskComponent = nullptr;
		NavigationComponent* navigationComponent = nullptr;
		TargetingComponent* targetingComponent = nullptr;
		TransformComponent* transformComponent = nullptr;

		bool AreComponentsValidCheck() const;
	};
	static void ProcessNavigationTaskCommands(TWeakObjectPtr<UWorld> worldPointer, const NavigationSystemsComponentArgs& components);
	static void NavigateFromEntityToLocation(TWeakObjectPtr<UWorld> worldPointer, FVector targetLocation, const NavigationSystemsComponentArgs& components);

private:
	static bool IsWorldPointerValidCheck(TWeakObjectPtr<UWorld> worldPointer);
};
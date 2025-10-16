// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/NavigationSystemsArgs.h"
#include <optional>

class NavigationSystems
{
public:
	static void RunSystems(UWorld* worldPointer);

	static void NavigateFromEntityToEntity(UWorld* worldPointer, ArgusEntity targetEntity, const NavigationSystemsArgs& components);
	static void NavigateFromEntityToLocation(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsArgs& components);

private:
	static void ProcessNavigationTaskCommands(UWorld* worldPointer, const NavigationSystemsArgs& components);
	static void RecalculateMoveToEntityPaths(UWorld* worldPointer, const NavigationSystemsArgs& components);
	static void ChangeTasksOnNavigatingToEntity(ArgusEntity targetEntity, const NavigationSystemsArgs& components);
	static void ChangeFlockingStateOnNavigatingToLocation(const NavigationSystemsArgs& components);
	static void GeneratePathPointsForGroundedEntity(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsArgs& components);
	static void GeneratePathPointsForFlyingEntity(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsArgs& components);
};
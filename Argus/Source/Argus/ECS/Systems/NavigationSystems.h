// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include <optional>

class NavigationSystems
{
public:
	static void RunSystems(UWorld* worldPointer);

	struct NavigationSystemsComponentArgs
	{
		TaskComponent* m_taskComponent = nullptr;
		NavigationComponent* m_navigationComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;
		const TransformComponent* m_transformComponent = nullptr;

		bool AreComponentsValidCheck() const;
	};
	static void ProcessNavigationTaskCommands(UWorld* worldPointer, const NavigationSystemsComponentArgs& components);
	static void RecalculateMoveToEntityPaths(UWorld* worldPointer, const NavigationSystemsComponentArgs& components);
	static void NavigateFromEntityToEntity(UWorld* worldPointer, ArgusEntity targetEntity, const NavigationSystemsComponentArgs& components);
	static void NavigateFromEntityToLocation(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsComponentArgs& components);

private:
	static bool IsWorldPointerValidCheck(UWorld* worldPointer);
};
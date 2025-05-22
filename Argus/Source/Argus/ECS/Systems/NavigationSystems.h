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
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		NavigationComponent* m_navigationComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;
		TransformComponent* m_transformComponent = nullptr;
		VelocityComponent* m_velocityComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};
	
	static void NavigateFromEntityToEntity(UWorld* worldPointer, ArgusEntity targetEntity, const NavigationSystemsComponentArgs& components);
	static void NavigateFromEntityToLocation(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsComponentArgs& components);

private:
	static void ProcessNavigationTaskCommands(UWorld* worldPointer, const NavigationSystemsComponentArgs& components);
	static void RecalculateMoveToEntityPaths(UWorld* worldPointer, const NavigationSystemsComponentArgs& components);
	static bool IsWorldPointerValidCheck(UWorld* worldPointer, const WIDECHAR* functionName);
	static void ChangeTasksOnNavigatingToEntity(ArgusEntity targetEntity, const NavigationSystemsComponentArgs& components);
};
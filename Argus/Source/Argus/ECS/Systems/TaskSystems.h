// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TaskSystemsArgs.h"

class TaskSystems
{
public:
	static void RunSystems(float deltaTime);

private:
	static void ProcessIdleEntity(const TaskSystemsArgs& components);
	static bool ProcessDispatchingForEntityPair(const TaskSystemsArgs& components, uint16 potentialTargetEntityId);
	static bool DispatchToConstructionIfAble(const TaskSystemsArgs& components, const ArgusEntity& potentialTargetEntity);
	static bool DispatchToCombatIfAble(const TaskSystemsArgs& components, const ArgusEntity& potentialTargetEntity);
};

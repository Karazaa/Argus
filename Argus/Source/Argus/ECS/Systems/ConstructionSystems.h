// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/ConstructionSystemsArgs.h"

class ConstructionSystems
{
public:
	static void RunSystems(float deltaTime);

	static bool CanEntityConstructOtherEntity(const ArgusEntity& potentialConstructor, const ArgusEntity& potentialConstructee);

private:
	static void ProcessConstructionTaskCommands(float deltaTime, const ConstructionSystemsArgs& components);
	static void ProcessBeingConstructedState(float deltaTime, const ConstructionSystemsArgs& components);
	static void ProcessConstructingOtherState(float deltaTime, const ConstructionSystemsArgs& components);
	static void ProcessAutomaticConstruction(float deltaTime, const ConstructionSystemsArgs& components);
	static void StopConstructingOther(const ConstructionSystemsArgs& components);
};
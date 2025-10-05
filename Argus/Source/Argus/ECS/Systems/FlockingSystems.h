// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/FlockingSystemsArgs.h"
#include "SystemArgumentDefinitions/TransformSystemsArgs.h"

class FlockingSystems
{
public:
	static void RunSystems(float deltaTime);
	static void ChooseFlockingRootEntityIfGroupLeader(const TransformSystemsArgs& components);
	static void EndFlockingIfNecessary(float deltaTime, const FlockingSystemsArgs& components);
};

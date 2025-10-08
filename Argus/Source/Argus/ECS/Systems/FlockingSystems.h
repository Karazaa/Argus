// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/FlockingSystemsArgs.h"
#include "SystemArgumentDefinitions/TransformSystemsArgs.h"

class FlockingSystems
{
public:
	static void RunSystems(float deltaTime);
	static void ChooseFlockingRootEntityIfGroupLeader(const TransformSystemsArgs& components);
	static ArgusEntity GetFlockingRootEntity(const ArgusEntity& entity);

private:
	static void EndFlockingIfNecessary(float deltaTime, const FlockingSystemsArgs& components);
	static void PackFlockingRoot(const FlockingSystemsArgs& components);
};

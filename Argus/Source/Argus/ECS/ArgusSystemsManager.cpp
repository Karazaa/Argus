// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TransformSystems.h"

void ArgusSystemsManager::RunSystems(float deltaTime)
{
	TargetingSystems::RunSystems(deltaTime);
	TransformSystems::RunSystems(deltaTime);
}
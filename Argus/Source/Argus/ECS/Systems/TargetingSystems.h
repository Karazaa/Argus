// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TargetingSystems
{
public:
	static void RunSystems(float deltaTime);

	static void TargetNearestEntityMatchingFactionMask(ArgusEntity sourceEntity, TransformComponent* sourceTransformComponent, uint8 factionMask);
};
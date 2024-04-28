// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TargetingSystems
{
public:
	static void RunSystems(float deltaTime);
	static void TargetNearestEntityMatchingFactionMask(const ArgusEntity& fromEntity, TransformComponent* fromTransformComponent, uint8 factionMask);
};
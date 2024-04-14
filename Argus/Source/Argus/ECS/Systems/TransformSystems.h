// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once
#include "../ArgusEntity.h"

class TransformSystems
{
public:
	static void RunSystems(const ArgusEntity& entity);
	static void FindNearestEntityAndTarget(const ArgusEntity& fromEntity, TransformComponent* fromTransformComponent);
};
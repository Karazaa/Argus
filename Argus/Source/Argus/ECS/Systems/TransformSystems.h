// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TransformSystems
{
public:
	static void RunSystems(float deltaTime);
	static void ProcessMovementTaskCommands(float deltaTime, ArgusEntity movingEntity, TransformComponent* moverTransformComponent);
	static void MoveAlongNavigationPath(float deltaTime, ArgusEntity movingEntity, TransformComponent* moverTransformComponent);
	static void FaceEntityTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget);
};
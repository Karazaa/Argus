// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TransformSystems
{
public:
	static void RunSystems(float deltaTime);

	struct TransformSystemsComponentArgs
	{
		TaskComponent* taskComponent = nullptr;
		TransformComponent* transformComponent = nullptr;
		NavigationComponent* navigationComponent = nullptr;

		bool AreComponentsValidCheck() const;
	};
	static void ProcessMovementTaskCommands(float deltaTime, const TransformSystemsComponentArgs& components);
	static void MoveAlongNavigationPath(float deltaTime, const TransformSystemsComponentArgs& components);
	static void FaceEntityTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget);
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TransformSystems
{
public:
	static void RunSystems(float deltaTime);

	struct TransformSystemsComponentArgs
	{
		TaskComponent* m_taskComponent = nullptr;
		TransformComponent* m_transformComponent = nullptr;
		NavigationComponent* m_navigationComponent = nullptr;

		bool AreComponentsValidCheck() const;
	};
	static void ProcessMovementTaskCommands(float deltaTime, const TransformSystemsComponentArgs& components);
	static void MoveAlongNavigationPath(float deltaTime, const TransformSystemsComponentArgs& components);
	static void FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget);
};
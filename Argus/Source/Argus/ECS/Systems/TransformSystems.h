// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TransformSystems
{
public:
	static bool RunSystems(float deltaTime);

	struct TransformSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::s_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		TransformComponent* m_transformComponent = nullptr;
		NavigationComponent* m_navigationComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;

		bool AreComponentsValidCheck() const;
	};
	static bool ProcessMovementTaskCommands(float deltaTime, const TransformSystemsComponentArgs& components);
	static void MoveAlongNavigationPath(float deltaTime, const TransformSystemsComponentArgs& components);
	static void FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget);
	static void FindEntitiesWithinXYBounds(FVector2D minXY, FVector2D maxXY, TArray<ArgusEntity>& outEntitiesWithinBounds);
	static void ProcessCollisions(float deltaTime, const TransformSystemsComponentArgs& components);

private:
	static void OnCompleteNavigationPath(const TransformSystemsComponentArgs& components);
	static void GetPathingLocationAtTimeOffset(float timeOffsetSeconds, const TransformSystemsComponentArgs& components, FVector& outputLocation, FVector& outputForwardVector, uint16& indexOfOutputLocation);
};
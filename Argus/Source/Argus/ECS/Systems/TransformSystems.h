// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TransformSystems
{
public:
	static bool RunSystems(UWorld* worldPointer, float deltaTime);

	struct TransformSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		TransformComponent* m_transformComponent = nullptr;
		NavigationComponent* m_navigationComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};
	struct GetPathingLocationAtTimeOffsetResults
	{
		FVector m_outputPredictedLocation;
		FVector m_outputPredictedForwardDirection;
		uint16  m_navigationIndexOfPredictedLocation;
	};
	static void MoveAlongNavigationPath(UWorld* worldPointer, float deltaTime, const TransformSystemsComponentArgs& components);

private:
	static bool ProcessMovementTaskCommands(UWorld* worldPointer, float deltaTime, const TransformSystemsComponentArgs& components);
	static void FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget);
	static void OnWithinRangeOfTargetEntity(const TransformSystemsComponentArgs& components);
	static void OnCompleteNavigationPath(const TransformSystemsComponentArgs& components, const FVector& moverLocation);
	static FVector ProjectLocationOntoNavigationData(UWorld* worldPointer, TransformComponent* transformComponent, const FVector& location);
	static float GetEndMoveRange(const TransformSystemsComponentArgs& components);
	static void UpdatePassengerLocations(const TransformSystemsComponentArgs& components);
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class TransformSystems
{
public:
	static bool RunSystems(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime);

	struct TransformSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::s_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		TransformComponent* m_transformComponent = nullptr;
		NavigationComponent* m_navigationComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;

		bool AreComponentsValidCheck() const;
	};
	struct GetPathingLocationAtTimeOffsetResults
	{
		FVector m_outputPredictedLocation;
		FVector m_outputPredictedForwardDirection;
		uint16  m_navigationIndexOfPredictedLocation;
	};

	static bool ProcessMovementTaskCommands(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime, const TransformSystemsComponentArgs& components);
	static void GetPathingLocationAtTimeOffset(float timeOffsetSeconds, const TransformSystemsComponentArgs& components, GetPathingLocationAtTimeOffsetResults& results);
	static void FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget);
	static void MoveAlongNavigationPath(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime, const TransformSystemsComponentArgs& components);
	static void FindEntitiesWithinXYBounds(FVector2D minXY, FVector2D maxXY, TArray<ArgusEntity>& outEntitiesWithinBounds);
	static void OnCompleteNavigationPath(const TransformSystemsComponentArgs& components);

	static FVector ProjectLocationOntoNavigationData(TWeakObjectPtr<UWorld>& worldPointer, const FVector& location);
};
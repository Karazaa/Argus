// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TransformSystemsArgs.h"

class TransformSystems
{
public:
	static bool RunSystems(UWorld* worldPointer, float deltaTime);

	struct GetPathingLocationAtTimeOffsetResults
	{
		FVector m_outputPredictedLocation;
		FVector m_outputPredictedForwardDirection;
		uint16  m_navigationIndexOfPredictedLocation;
	};
	static void MoveAlongNavigationPath(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components);

private:
	static bool ProcessMovementTaskCommands(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components);
	static void ProcessTakeOffCommand(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components);
	static void ProcessLandCommand(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components);
	static void FaceTargetEntity(const TransformSystemsArgs& components);
	static void FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget);
	static void OnWithinRangeOfTargetEntity(const TransformSystemsArgs& components);
	static void OnCompleteNavigationPath(const TransformSystemsArgs& components, const FVector& moverLocation);
	static FVector ProjectLocationOntoNavigationData(UWorld* worldPointer, TransformComponent* transformComponent, const FVector& location);
	static float GetEndMoveRange(const TransformSystemsArgs& components);
	static void UpdatePassengerLocations(const TransformSystemsArgs& components);
	static void ShowTraceForFlyingEntity(UWorld* worldPointer, const TransformSystemsArgs& components);
};
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
	static FVector ProjectLocationOntoNavigationData(UWorld* worldPointer, float navAgentRadius, const FVector& location);

	static float GetDesiredSpeed(const TaskComponent* taskComponent, const VelocityComponent* velocityComponent);
	static float GetDesiredSpeed(ArgusEntity entity);
	static float GetEndMoveRange(ArgusEntity entity, const TaskComponent* taskComponent, const TargetingComponent* targetingComponent, const TransformComponent* transformComponent);
	static bool  IsWithinEndMoveRange(ArgusEntity entity, const TaskComponent* taskComponent, const TargetingComponent* targetingComponent, const TransformComponent* transformComponent);

private:
	static bool ProcessMovementTaskCommands(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components);
	static void ProcessTakeOffCommand(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components);
	static void ProcessLandCommand(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components);

	static void PerformTakeOff(float deltaTime, const TransformSystemsArgs& components);
	static void PerformLanding(float deltaTime, const TransformSystemsArgs& components);

	static void FaceTargetEntity(const TransformSystemsArgs& components);
	static void FaceVelocity(const TransformSystemsArgs& components);
	static void FaceTowardsLocationXY(FacingComponent* facingComponent, FVector vectorFromTransformToTarget);

	static bool CheckFlockCompletedNavigation(const TransformSystemsArgs& components);
	static void OnWithinRangeOfTargetEntity(const TransformSystemsArgs& components);

	static bool ShouldAdvanceSegment(const FVector& evaluationPoint, const FVector& segmentStart, const FVector& segmentEnd);
	static bool UpdateGroupIndex(const TransformSystemsArgs& components, const FVector& velocity);
	static bool IsGroupNearCompletion(ArgusEntity groupLeader, const FVector& velocity);
	static void UpdateIndividualSegmentIndex(NavigationComponent* navigationComponent, const FVector& moverLocation, const TArray<FVector, ArgusContainerAllocator<15u> >& navigationPoints);

	static void OnCompleteNavigationPath(const TransformSystemsArgs& components);
	static void CompleteNavigationPathPerGroupEntity(ArgusEntity entity);
	static void UpdatePassengerLocations(const TransformSystemsArgs& components);
	static void ShowTraceForFlyingEntity(UWorld* worldPointer, const TransformSystemsArgs& components);
};
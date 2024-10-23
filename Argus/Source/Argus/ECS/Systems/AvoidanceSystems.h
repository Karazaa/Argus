// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "TransformSystems.h"

class AvoidanceSystems
{
public:
	static void RunSystems(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime);
	
private:

#pragma region Optimal Reciprocal Collision Avoidance
	struct ORCALine
	{
		FVector2D m_direction = FVector2D::ZeroVector;
		FVector2D m_point = FVector2D::ZeroVector;
	};
	struct FindORCALineParams
	{
		FVector2D m_sourceEntityLocation = FVector2D::ZeroVector;
		FVector2D m_sourceEntityVelocity = FVector2D::ZeroVector;
		FVector2D m_foundEntityLocation = FVector2D::ZeroVector;
		FVector2D m_foundEntityVelocity = FVector2D::ZeroVector;
		float m_deltaTime = 0.0f;
		float m_inversePredictionTime = 0.0f;
	};

	static void ProcessORCAvoidance(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components);
	static void FindORCALineAndVelocityToBoundaryPerEntity(const FindORCALineParams& findOrcaLineParams, FVector2D& velocityToBoundaryOfVO, ORCALine& orcaLine);
	static bool OneDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int lineIndex, FVector2D& resultingVelocity);
	static bool TwoDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int& failureLine);
	static void ThreeDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const int lineIndex, FVector2D& resultingVelocity);
#pragma endregion

#pragma region Non-ORC Avoidance
	static void ProcessPotentialCollisions(float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components);
	static void HandlePotentialCollisionWithMovableEntity(const TransformSystems::GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystems::TransformSystemsComponentArgs& components, const TransformSystems::TransformSystemsComponentArgs& otherEntityComponents);
	static void HandlePotentialCollisionWithStaticEntity(const TransformSystems::GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystems::TransformSystemsComponentArgs& components, const TransformComponent* otherEntityTransformComponent);
	static FVector GetAvoidancePointLocationForStaticCollision(const FVector& avoidingAgentLocation, const FVector& obstacleLocation, const FVector& predictedCollisionLocation);
#pragma endregion

};
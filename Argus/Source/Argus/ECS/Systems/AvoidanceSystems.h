// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "TransformSystems.h"

class AvoidanceSystems
{
public:
	static void RunSystems(float deltaTime);
	
private:

#pragma region RVO avoidance
	static void ProcessRVOAvoidance(float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components);
#pragma endregion

#pragma region Non-RVO avoidance
	static void ProcessPotentialCollisions(float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components);
	static void HandlePotentialCollisionWithMovableEntity(const TransformSystems::GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystems::TransformSystemsComponentArgs& components, const TransformSystems::TransformSystemsComponentArgs& otherEntityComponents);
	static void HandlePotentialCollisionWithStaticEntity(const TransformSystems::GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystems::TransformSystemsComponentArgs& components, const TransformComponent* otherEntityTransformComponent);
	static FVector GetAvoidancePointLocationForStaticCollision(const FVector& avoidingAgentLocation, const FVector& obstacleLocation, const FVector& predictedCollisionLocation);
#pragma endregion

};
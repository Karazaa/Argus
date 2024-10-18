// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AvoidanceSystems.h"
#include "ArgusSystemsManager.h"

void AvoidanceSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(AvoidanceSystems::RunSystems)

	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		TransformSystems::TransformSystemsComponentArgs components;
		components.m_entity = potentialEntity;
		components.m_taskComponent = potentialEntity.GetComponent<TaskComponent>();
		components.m_transformComponent = potentialEntity.GetComponent<TransformComponent>();
		components.m_navigationComponent = potentialEntity.GetComponent<NavigationComponent>();
		components.m_targetingComponent = potentialEntity.GetComponent<TargetingComponent>();
		if (!components.AreComponentsValidCheck())
		{
			continue;
		}

		if (components.m_taskComponent->IsExecutingMoveTask())
		{
			ProcessRVOAvoidance(deltaTime, components);
			ProcessPotentialCollisions(deltaTime, components);
		}
	}
}

#pragma region RVO Avoidance
void AvoidanceSystems::ProcessRVOAvoidance(float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	const ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusSystemsManager::s_singletonEntityId);
	if (!singletonEntity)
	{
		return;
	}

	const SpatialPartitioningComponent* const spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	// TODO James: Need to calculate avoidance speed at the end in the final version. This is just temp for now. Need to remove this line later.
	components.m_transformComponent->m_avoidanceSpeedUnitsPerSecond = components.m_transformComponent->m_desiredSpeedUnitsPerSecond;

	const FVector2D sourceEntityLocation = FVector2D(components.m_transformComponent->m_transform.GetLocation());
	const FVector2D sourceEntityVelocity = FVector2D(components.m_transformComponent->m_transform.GetRotation().GetForwardVector()) * components.m_transformComponent->m_desiredSpeedUnitsPerSecond * ArgusECSConstants::k_defaultCollisionDetectionPredictionTime;
	const float inversePredictionTime = 1.0F / ArgusECSConstants::k_defaultCollisionDetectionPredictionTime;

	std::vector<uint16> foundEntityIds;
	if (spatialPartitioningComponent->m_argusKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(foundEntityIds, components.m_entity, ArgusECSConstants::k_defaultAvoidanceAgentRadius))
	{
		for (int i = 0; i < foundEntityIds.size(); ++i)
		{
			ArgusEntity foundEntity = ArgusEntity::RetrieveEntity(foundEntityIds[i]);
			if (!foundEntity)
			{
				continue;
			}

			TransformComponent* foundTransformComponent = foundEntity.GetComponent<TransformComponent>();
			if (!foundTransformComponent)
			{
				continue;
			}

			const FVector2D foundEntityLocation = FVector2D(foundTransformComponent->m_transform.GetLocation());
			FVector2D foundEntityVelocity = FVector2D(foundTransformComponent->m_transform.GetRotation().GetForwardVector()) * foundTransformComponent->m_desiredSpeedUnitsPerSecond * ArgusECSConstants::k_defaultCollisionDetectionPredictionTime;

			TaskComponent* foundTaskComponent = foundEntity.GetComponent<TaskComponent>();
			if (!foundTaskComponent || !foundTaskComponent->IsExecutingMoveTask())
			{
				foundEntityVelocity = FVector2D::ZeroVector;
			}

			const FVector2D foundEntityLocationRelativeToSource = foundEntityLocation - sourceEntityLocation;
			const FVector2D foundEntityVelocityRelativeToSource = foundEntityVelocity - sourceEntityVelocity;
			const float relativeLocationDistanceSquared = foundEntityLocationRelativeToSource.SquaredLength();
			const float combinedRadius = 2.0f * ArgusECSConstants::k_defaultAvoidanceAgentRadius;
			const float combinedRadiusSquared = combinedRadius * combinedRadius;

			FVector2D velocityToBoundaryOfVO = FVector2D::ZeroVector;
			FVector2D orcaLineDirection = FVector2D::ZeroVector;
			if (relativeLocationDistanceSquared > combinedRadiusSquared)
			{
				// No collision yet.
				FVector2D cutoffCenterToRelativeVelocity = foundEntityVelocityRelativeToSource - (inversePredictionTime * foundEntityLocationRelativeToSource);

				// TODO JAMES: Need more here.
			}
			else
			{
				// Collision occurred.
				const float inverseDeltaTime = 1.0f / deltaTime;

				const FVector2D cutoffCenterToRelativeVelocity = foundEntityVelocityRelativeToSource - (inverseDeltaTime * foundEntityLocationRelativeToSource);
				const float lengthCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity.Length();
				const FVector2D normalizedCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity / lengthCutoffCenterToRelativeVelocity;

				orcaLineDirection = FVector2D(normalizedCutoffCenterToRelativeVelocity.Y, -normalizedCutoffCenterToRelativeVelocity.X);
				velocityToBoundaryOfVO = ((combinedRadius * inverseDeltaTime) - lengthCutoffCenterToRelativeVelocity) * normalizedCutoffCenterToRelativeVelocity;
			}
			
			// TODO JAMES: https://github.com/snape/RVO2/blob/af26bedf27a84ffffb59beea996ffe2531ddc789/src/Agent.cc#L541
		}
	}
}
#pragma endregion

#pragma region Non-RVO avoidance
void AvoidanceSystems::ProcessPotentialCollisions(float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	const ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusSystemsManager::s_singletonEntityId);
	if (!singletonEntity)
	{
		return;
	}

	const SpatialPartitioningComponent* const spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	TransformSystems::GetPathingLocationAtTimeOffsetResults results;
	results.m_outputPredictedLocation = components.m_transformComponent->m_transform.GetLocation();
	results.m_outputPredictedForwardDirection = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
	results.m_navigationIndexOfPredictedLocation = components.m_navigationComponent->m_lastPointIndex;
	TransformSystems::GetPathingLocationAtTimeOffset(ArgusECSConstants::k_defaultCollisionDetectionPredictionTime, components, results);

	const uint16 nearestOtherEntityId = spatialPartitioningComponent->m_argusKDTree.FindArgusEntityIdClosestToLocation(results.m_outputPredictedLocation, components.m_entity);
	if (nearestOtherEntityId == ArgusECSConstants::k_maxEntities)
	{
		return;
	}

	ArgusEntity nearestOtherEntity = ArgusEntity::RetrieveEntity(nearestOtherEntityId);
	if (!nearestOtherEntity)
	{
		return;
	}

	TransformComponent* nearestOtherEntityTransformComponent = nearestOtherEntity.GetComponent<TransformComponent>();
	if (!nearestOtherEntityTransformComponent)
	{
		return;
	}

	TransformSystems::TransformSystemsComponentArgs nearestOtherEntityComponents;
	nearestOtherEntityComponents.m_entity = nearestOtherEntity;
	nearestOtherEntityComponents.m_navigationComponent = nearestOtherEntity.GetComponent<NavigationComponent>();
	nearestOtherEntityComponents.m_targetingComponent = nearestOtherEntity.GetComponent<TargetingComponent>();
	nearestOtherEntityComponents.m_taskComponent = nearestOtherEntity.GetComponent<TaskComponent>();
	nearestOtherEntityComponents.m_transformComponent = nearestOtherEntityTransformComponent;

	if (nearestOtherEntityComponents.AreComponentsValidCheck())
	{
		HandlePotentialCollisionWithMovableEntity(results, components, nearestOtherEntityComponents);
	}
	else
	{
		HandlePotentialCollisionWithStaticEntity(results, components, nearestOtherEntityTransformComponent);
	}
}

void AvoidanceSystems::HandlePotentialCollisionWithMovableEntity(const TransformSystems::GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystems::TransformSystemsComponentArgs& components, const TransformSystems::TransformSystemsComponentArgs& otherEntityComponents)
{
	if (!components.AreComponentsValidCheck() || !otherEntityComponents.AreComponentsValidCheck())
	{
		return;
	}

	TransformSystems::GetPathingLocationAtTimeOffsetResults results;
	results.m_outputPredictedLocation = otherEntityComponents.m_transformComponent->m_transform.GetLocation();
	results.m_outputPredictedForwardDirection = otherEntityComponents.m_transformComponent->m_transform.GetRotation().GetForwardVector();
	results.m_navigationIndexOfPredictedLocation = otherEntityComponents.m_navigationComponent->m_lastPointIndex;
	TransformSystems::GetPathingLocationAtTimeOffset(ArgusECSConstants::k_defaultCollisionDetectionPredictionTime, otherEntityComponents, results);

	const float distanceSquared = FVector::DistSquared(movingEntityPredictedMovement.m_outputPredictedLocation, results.m_outputPredictedLocation);
	if (distanceSquared > FMath::Square(ArgusECSConstants::k_defaultPathFindingAgentRadius))
	{
		return;
	}

	UE_LOG(ArgusECSLog, Display, TEXT("[%s] Transform Systems detected that %s %d will collide with %s %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), components.m_entity.GetId(), ARGUS_NAMEOF(ArgusEntity), otherEntityComponents.m_entity.GetId());

	if (otherEntityComponents.m_taskComponent->IsExecutingMoveTask())
	{
		// TODO JAMES: Two movers colliding, gotta figure that out!
		return;
	}

	const FVector navigationPointPriorToCollision = otherEntityComponents.m_transformComponent->m_transform.GetLocation();
	const FVector otherEntityLocation = components.m_transformComponent->m_transform.GetLocation();
	const FVector inversePredictedLocation = navigationPointPriorToCollision - (movingEntityPredictedMovement.m_outputPredictedLocation - otherEntityLocation);
	const FVector avoidancePointLocation = GetAvoidancePointLocationForStaticCollision(navigationPointPriorToCollision, otherEntityLocation, inversePredictedLocation);

	otherEntityComponents.m_taskComponent->m_currentTask = ETask::MoveToLocation;
	otherEntityComponents.m_navigationComponent->m_lastPointIndex = 0u;
	otherEntityComponents.m_navigationComponent->m_navigationPoints.push_back(navigationPointPriorToCollision);
	otherEntityComponents.m_navigationComponent->m_navigationPoints.push_back(avoidancePointLocation);
}

void AvoidanceSystems::HandlePotentialCollisionWithStaticEntity(const TransformSystems::GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystems::TransformSystemsComponentArgs& components, const TransformComponent* otherEntityTransformComponent)
{
	if (!components.AreComponentsValidCheck() || !components.m_taskComponent->IsExecutingMoveTask() || !otherEntityTransformComponent)
	{
		return;
	}

	const uint16 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.size();
	if (numNavigationPoints == 0u)
	{
		return;
	}

	const FVector endNavigationLocation = components.m_navigationComponent->m_navigationPoints[numNavigationPoints - 1u];
	const FVector otherEntityLocation = otherEntityTransformComponent->m_transform.GetLocation();

	const float currentDistanceSquared = FVector::DistSquared(movingEntityPredictedMovement.m_outputPredictedLocation, otherEntityLocation);
	if (currentDistanceSquared > FMath::Square(ArgusECSConstants::k_defaultPathFindingAgentRadius))
	{
		return;
	}

	UE_LOG(ArgusECSLog, Display, TEXT("[%s] Transform Systems detected that %s %d will collide with a static %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), components.m_entity.GetId(), ARGUS_NAMEOF(ArgusEntity));

	const float goalDistanceSquared = FVector::DistSquared(endNavigationLocation, otherEntityTransformComponent->m_transform.GetLocation());
	if (goalDistanceSquared <= FMath::Square(ArgusECSConstants::k_defaultPathFindingAgentRadius))
	{
		TransformSystems::FaceTowardsLocationXY(components.m_transformComponent, movingEntityPredictedMovement.m_outputPredictedLocation - components.m_transformComponent->m_transform.GetLocation());
		TransformSystems::OnCompleteNavigationPath(components);
		return;
	}

	const FVector navigationPointPriorToCollision = components.m_navigationComponent->m_navigationPoints[movingEntityPredictedMovement.m_navigationIndexOfPredictedLocation];
	const FVector avoidancePointLocation = GetAvoidancePointLocationForStaticCollision(navigationPointPriorToCollision, otherEntityLocation, movingEntityPredictedMovement.m_outputPredictedLocation);

	components.m_navigationComponent->m_navigationPoints.insert(components.m_navigationComponent->m_navigationPoints.begin() + movingEntityPredictedMovement.m_navigationIndexOfPredictedLocation + 1u, avoidancePointLocation);
}

FVector AvoidanceSystems::GetAvoidancePointLocationForStaticCollision(const FVector& avoidingAgentLocation, const FVector& obstacleLocation, const FVector& predictedCollisionLocation)
{
	const FVector positionDeltaObstacle = obstacleLocation - avoidingAgentLocation;
	const FVector positionDeltaMoveDirection = predictedCollisionLocation - avoidingAgentLocation;

	const float signAngle = FMath::Sign(positionDeltaMoveDirection.Cross(positionDeltaObstacle).Z);
	const float lengthPositionDeltaObstacle = positionDeltaObstacle.Length();
	const float angleToAvoidancePoint = FMath::Asin((ArgusECSConstants::k_defaultPathFindingAgentRadius + ArgusECSConstants::k_defaultPathFindingAgentAvoidanceCushion) / lengthPositionDeltaObstacle);
	const float lengthToAvoidancePoint = FMath::Max(FMath::Cos(angleToAvoidancePoint) * lengthPositionDeltaObstacle, ArgusECSConstants::k_defaultPathFindingAgentAvoidanceCushion);

	const FVector positionDeltaAvoidancePoint = positionDeltaObstacle.RotateAngleAxisRad(angleToAvoidancePoint * -signAngle, FVector::UpVector).GetSafeNormal() * lengthToAvoidancePoint;
	return avoidingAgentLocation + positionDeltaAvoidancePoint;
}
#pragma endregion

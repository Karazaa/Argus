// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"
#include "ArgusSystemsManager.h"
#include "Math/UnrealMathUtility.h"

bool TransformSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TransformSystems::RunSystems)

	bool didMovementUpdateThisFrame = false;
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		TransformSystemsComponentArgs components;
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
			ProcessCollisions(deltaTime, components);
		}

		didMovementUpdateThisFrame |= ProcessMovementTaskCommands(deltaTime, components);
	}

	return didMovementUpdateThisFrame;
}

bool TransformSystems::TransformSystemsComponentArgs::AreComponentsValidCheck() const
{
	return m_entity && m_taskComponent && m_navigationComponent && m_transformComponent && m_targetingComponent;
}

bool TransformSystems::ProcessMovementTaskCommands(float deltaTime, const TransformSystemsComponentArgs& components)
{
	ARGUS_TRACE(TransformSystems::ProcessMovementTaskCommands)

	if (!components.AreComponentsValidCheck())
	{
		return false;
	}

	switch (components.m_taskComponent->m_currentTask)
	{
		case ETask::MoveToLocation:
		case ETask::MoveToEntity:
			MoveAlongNavigationPath(deltaTime, components);
			return true;
		default:
			return false;
	}
}

void TransformSystems::GetPathingLocationAtTimeOffset(float timeOffsetSeconds, const TransformSystemsComponentArgs& components, GetPathingLocationAtTimeOffsetResults& results)
{
	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	if (!components.m_taskComponent->IsExecutingMoveTask())
	{
		results.m_outputPredictedLocation = components.m_transformComponent->m_transform.GetLocation();
		results.m_outputPredictedForwardDirection = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
		return;
	}

	uint16 pointIndex = components.m_navigationComponent->m_lastPointIndex;
	const uint16 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.size();

	if (pointIndex >= numNavigationPoints - 1u)
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
		return;
	}

	FVector currentLocation = components.m_transformComponent->m_transform.GetLocation();

	if (timeOffsetSeconds == 0.0f)
	{
		results.m_outputPredictedLocation = currentLocation;
		results.m_outputPredictedForwardDirection = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
		results.m_navigationIndexOfPredictedLocation = pointIndex;
		return;
	}

	float translationMagnitude = FMath::Abs(components.m_transformComponent->m_speedUnitsPerSecond * timeOffsetSeconds);
	FVector positionDifferenceNormalized = FVector::ZeroVector;

	if (timeOffsetSeconds > 0.0f)
	{
		FVector upcomingPoint = components.m_navigationComponent->m_navigationPoints[pointIndex + 1u];
		FVector positionDifference = upcomingPoint - currentLocation;
		float positionDifferenceMagnitude = positionDifference.Length();
		positionDifferenceNormalized = positionDifference.GetSafeNormal();

		while (translationMagnitude >= positionDifferenceMagnitude)
		{
			pointIndex++;
			if (pointIndex >= numNavigationPoints - 1u)
			{
				results.m_outputPredictedLocation = components.m_navigationComponent->m_navigationPoints[pointIndex];
				results.m_navigationIndexOfPredictedLocation = pointIndex;
				return;
			}

			translationMagnitude -= positionDifferenceMagnitude;
			upcomingPoint = components.m_navigationComponent->m_navigationPoints[pointIndex + 1u];
			currentLocation = components.m_navigationComponent->m_navigationPoints[pointIndex];
			positionDifference = upcomingPoint - currentLocation;
			positionDifferenceMagnitude = positionDifference.Length();
			positionDifferenceNormalized = positionDifference.GetSafeNormal();
		}

		results.m_outputPredictedForwardDirection = positionDifferenceNormalized;
	}
	else
	{
		FVector upcomingPoint = components.m_navigationComponent->m_navigationPoints[pointIndex];
		FVector positionDifference = upcomingPoint - currentLocation;
		float positionDifferenceMagnitude = positionDifference.Length();
		positionDifferenceNormalized = positionDifference.GetSafeNormal();

		while (translationMagnitude > positionDifferenceMagnitude)
		{
			if (pointIndex == 0u)
			{
				results.m_outputPredictedLocation = components.m_navigationComponent->m_navigationPoints[pointIndex];
				results.m_navigationIndexOfPredictedLocation = 0u;
				return;
			}
			pointIndex--;
			translationMagnitude -= positionDifferenceMagnitude;

			upcomingPoint = components.m_navigationComponent->m_navigationPoints[pointIndex];
			currentLocation = components.m_navigationComponent->m_navigationPoints[pointIndex + 1];
			positionDifference = upcomingPoint - currentLocation;
			positionDifferenceMagnitude = positionDifference.Length();
			positionDifferenceNormalized = positionDifference.GetSafeNormal();
		}

		results.m_outputPredictedForwardDirection = -positionDifferenceNormalized;
	}

	results.m_outputPredictedLocation = currentLocation + (translationMagnitude * positionDifferenceNormalized);
	results.m_navigationIndexOfPredictedLocation = pointIndex;
}

void TransformSystems::FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget)
{
	if (vectorFromTransformToTarget.Equals(FVector::ZeroVector))
	{
		return;
	}

	if (!transformComponent)
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] Transform Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return;
	}

	vectorFromTransformToTarget.Z = 0.0f;
	if (vectorFromTransformToTarget.GetSafeNormal().Equals(transformComponent->m_transform.GetRotation().GetForwardVector()))
	{
		return;
	}

	transformComponent->m_transform.SetRotation(FRotationMatrix::MakeFromXZ(vectorFromTransformToTarget, FVector::UpVector).ToQuat());
}

void TransformSystems::MoveAlongNavigationPath(float deltaTime, const TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	const uint16 lastPointIndex = components.m_navigationComponent->m_lastPointIndex;
	const uint16 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.size();

	if (lastPointIndex >= numNavigationPoints - 1u)
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
		return;
	}

	GetPathingLocationAtTimeOffsetResults results;
	results.m_outputPredictedLocation = components.m_transformComponent->m_transform.GetLocation();
	results.m_outputPredictedForwardDirection = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
	results.m_navigationIndexOfPredictedLocation = lastPointIndex;
	GetPathingLocationAtTimeOffset(deltaTime, components, results);

	FaceTowardsLocationXY(components.m_transformComponent, results.m_outputPredictedForwardDirection);

	components.m_transformComponent->m_transform.SetLocation(results.m_outputPredictedLocation);
	components.m_navigationComponent->m_lastPointIndex = results.m_navigationIndexOfPredictedLocation;
	if (components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 1u)
	{
		OnCompleteNavigationPath(components);
	}
}

void TransformSystems::ProcessCollisions(float deltaTime, const TransformSystemsComponentArgs& components)
{
	const ArgusEntity singletonEntity = ArgusEntity::RetrieveEntity(ArgusSystemsManager::s_singletonEntityId);
	if (!singletonEntity)
	{
		return;
	}

	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	const SpatialPartitioningComponent* const spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	GetPathingLocationAtTimeOffsetResults results;
	results.m_outputPredictedLocation = components.m_transformComponent->m_transform.GetLocation();
	results.m_outputPredictedForwardDirection = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
	results.m_navigationIndexOfPredictedLocation = components.m_navigationComponent->m_lastPointIndex;
	GetPathingLocationAtTimeOffset(ArgusECSConstants::k_defaultCollisionDetectionPredictionTime, components, results);

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

	TransformSystemsComponentArgs nearestOtherEntityComponents;
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

void TransformSystems::FindEntitiesWithinXYBounds(FVector2D minXY, FVector2D maxXY, TArray<ArgusEntity>& outEntitiesWithinBounds)
{
	ARGUS_TRACE(TransformSystems::FindEntitiesWithinXYBounds)

	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		TransformComponent* transformComponent = potentialEntity.GetComponent<TransformComponent>();
		if (!transformComponent)
		{
			continue;
		}

		FVector location = transformComponent->m_transform.GetLocation();

		if (location.X >= minXY.X &&
			location.X <= maxXY.X &&
			location.Y >= minXY.Y &&
			location.Y <= maxXY.Y)
		{
			outEntitiesWithinBounds.Emplace(potentialEntity);
		}
	}
}

void TransformSystems::OnCompleteNavigationPath(const TransformSystemsComponentArgs& components)
{
	if (components.m_navigationComponent->m_queuedWaypoints.size() == 0u)
	{
		components.m_taskComponent->m_currentTask = ETask::None;
		components.m_navigationComponent->ResetPath();
	}
	else
	{
		components.m_taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
		components.m_navigationComponent->ResetPath();
		components.m_targetingComponent->m_targetLocation = components.m_navigationComponent->m_queuedWaypoints.front();
		components.m_navigationComponent->m_queuedWaypoints.pop();
	}
}

void TransformSystems::HandlePotentialCollisionWithMovableEntity(const GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystemsComponentArgs& components, const TransformSystemsComponentArgs& otherEntityComponents)
{
	if (!components.AreComponentsValidCheck() || !otherEntityComponents.AreComponentsValidCheck())
	{
		return;
	}

	GetPathingLocationAtTimeOffsetResults results;
	results.m_outputPredictedLocation = otherEntityComponents.m_transformComponent->m_transform.GetLocation();
	results.m_outputPredictedForwardDirection = otherEntityComponents.m_transformComponent->m_transform.GetRotation().GetForwardVector();
	results.m_navigationIndexOfPredictedLocation = otherEntityComponents.m_navigationComponent->m_lastPointIndex;
	GetPathingLocationAtTimeOffset(ArgusECSConstants::k_defaultCollisionDetectionPredictionTime, otherEntityComponents, results);

	const float distanceSquared = FVector::DistSquared(movingEntityPredictedMovement.m_outputPredictedLocation, results.m_outputPredictedLocation);
	if (distanceSquared < FMath::Square(ArgusECSConstants::k_defaultPathFindingAgentRadius))
	{
		UE_LOG(ArgusECSLog, Display, TEXT("[%s] Transform Systems detected that %s %d will collide with %s %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), components.m_entity.GetId(), ARGUS_NAMEOF(ArgusEntity), otherEntityComponents.m_entity.GetId());

		if (otherEntityComponents.m_taskComponent->IsExecutingMoveTask())
		{
			// TODO JAMES: Two movers colliding, gotta figure that out!
			return;
		}

		// TODO JAMES: Mover colliding with a movable static entity, have to process avoidance from the perspective of the currently static entity.
	}
}

void TransformSystems::HandlePotentialCollisionWithStaticEntity(const GetPathingLocationAtTimeOffsetResults& movingEntityPredictedMovement, const TransformSystemsComponentArgs& components, const TransformComponent* otherEntityTransformComponent)
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

	const float goalDistanceSquared = FVector::DistSquared(endNavigationLocation, otherEntityTransformComponent->m_transform.GetLocation());
	if (goalDistanceSquared <= FMath::Square(ArgusECSConstants::k_defaultPathFindingAgentRadius))
	{
		UE_LOG(ArgusECSLog, Display, TEXT("[%s] Transform Systems detected that %s %d will collide with a static %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), components.m_entity.GetId(), ARGUS_NAMEOF(ArgusEntity));
		FaceTowardsLocationXY(components.m_transformComponent, movingEntityPredictedMovement.m_outputPredictedLocation - components.m_transformComponent->m_transform.GetLocation());
		OnCompleteNavigationPath(components);
		return;
	}

	const FVector navigationPointPriorToCollision = components.m_navigationComponent->m_navigationPoints[movingEntityPredictedMovement.m_navigationIndexOfPredictedLocation];
	const FVector positionDeltaObstacle = otherEntityLocation - navigationPointPriorToCollision;
	const FVector positionDeltaMoveDirection = movingEntityPredictedMovement.m_outputPredictedLocation - navigationPointPriorToCollision;

	const float signAngle = FMath::Sign(positionDeltaMoveDirection.Cross(positionDeltaObstacle).Z);
	const float lengthPositionDeltaObstacle = positionDeltaObstacle.Length();
	const float angleToAvoidancePoint = FMath::Asin((ArgusECSConstants::k_defaultPathFindingAgentRadius + ArgusECSConstants::k_defaultPathFindingAgentAvoidanceCushion)  / lengthPositionDeltaObstacle);
	const float lengthToAvoidancePoint = FMath::Max(FMath::Cos(angleToAvoidancePoint) * lengthPositionDeltaObstacle, ArgusECSConstants::k_defaultPathFindingAgentAvoidanceCushion);

	const FVector positionDeltaAvoidancePoint = positionDeltaObstacle.RotateAngleAxisRad(angleToAvoidancePoint * -signAngle, FVector::UpVector).GetSafeNormal() * lengthToAvoidancePoint;
	const FVector avoidancePointLocation = navigationPointPriorToCollision + positionDeltaAvoidancePoint;

	components.m_navigationComponent->m_navigationPoints.insert(components.m_navigationComponent->m_navigationPoints.begin() + movingEntityPredictedMovement.m_navigationIndexOfPredictedLocation + 1u, avoidancePointLocation);
}
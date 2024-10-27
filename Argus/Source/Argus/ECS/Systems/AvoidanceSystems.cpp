// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AvoidanceSystems.h"
#include "ArgusMath.h"
#include "ArgusSystemsManager.h"

static TAutoConsoleVariable<bool> CVarShowAvoidanceDebug(TEXT("Argus.Avoidance.ShowAvoidanceDebug"), false, TEXT(""));

void AvoidanceSystems::RunSystems(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime)
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

		// TODO James: Need to calculate avoidance speed at the end in the final version. This is just temp for now. Need to remove this line later.
		components.m_transformComponent->m_avoidanceSpeedUnitsPerSecond = components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
		ProcessORCAvoidance(worldPointer, deltaTime, components);

		// TODO James: Consider deleting along with all non RVO avoidance once RVO implementation is ossified.
		//if (components.m_taskComponent->IsExecutingMoveTask())
		//{
		//	ProcessPotentialCollisions(deltaTime, components);
		//}
	}
}

#pragma region Optimal Reciprocal Collision Avoidance
void AvoidanceSystems::ProcessORCAvoidance(TWeakObjectPtr<UWorld>& worldPointer, float deltaTime, const TransformSystems::TransformSystemsComponentArgs& components)
{
	ARGUS_TRACE(AvoidanceSystems::ProcessORCAvoidance)

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

	const FVector2D sourceEntityLocation = FVector2D(components.m_transformComponent->m_transform.GetLocation());
	const FVector2D sourceEntityVelocity = FVector2D(components.m_transformComponent->m_avoidanceVelocity);
	FVector2D desiredVelocity = FVector2D::ZeroVector;
	if (components.m_taskComponent->IsExecutingMoveTask())
	{
		const uint16 futureIndex = components.m_navigationComponent->m_lastPointIndex + 1u;
		FVector desiredDirection = FVector::ZeroVector;
		if (futureIndex < components.m_navigationComponent->m_navigationPoints.size())
		{
			desiredDirection = (components.m_navigationComponent->m_navigationPoints[futureIndex] - components.m_transformComponent->m_transform.GetLocation()).GetSafeNormal();
		}
		else
		{
			UE_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to process ORCA, but the source %s's %s is in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(NavigationComponent));
			desiredDirection = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
		}

		desiredVelocity = FVector2D(desiredDirection) * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
	}

	const float inversePredictionTime = 1.0f / ArgusECSConstants::k_defaultCollisionDetectionPredictionTime;

	std::vector<uint16> foundEntityIds;
	if (!spatialPartitioningComponent->m_argusKDTree.FindOtherArgusEntityIdsWithinRangeOfArgusEntity(foundEntityIds, components.m_entity, ArgusECSConstants::k_defaultAvoidanceAgentSearchRadius))
	{
		components.m_transformComponent->m_avoidanceVelocity = FVector(desiredVelocity, 0.0f);
		if (CVarShowAvoidanceDebug.GetValueOnGameThread() && worldPointer.IsValid())
		{
			FVector sourceEntityLocation3D = FVector(sourceEntityLocation, 5.0f);
			DrawDebugLine(worldPointer.Get(), sourceEntityLocation3D, sourceEntityLocation3D + FVector(sourceEntityVelocity, 0.0f), FColor::Orange, false, -1.0f, 0, 5.0f);
		}
		return;
	}

	std::vector<ORCALine> orcaLines;
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

		FindORCALineParams params;
		params.m_sourceEntityLocation = sourceEntityLocation;
		params.m_sourceEntityVelocity = sourceEntityVelocity;
		params.m_deltaTime = deltaTime;
		params.m_inversePredictionTime = inversePredictionTime;

		params.m_foundEntityLocation = FVector2D(foundTransformComponent->m_transform.GetLocation());
		params.m_foundEntityVelocity = FVector2D::ZeroVector;

		TaskComponent* foundTaskComponent = foundEntity.GetComponent<TaskComponent>();
		if (foundTaskComponent && foundTaskComponent->IsExecutingMoveTask())
		{
			params.m_foundEntityVelocity = FVector2D(foundTransformComponent->m_transform.GetRotation().GetForwardVector()) * foundTransformComponent->m_desiredSpeedUnitsPerSecond * ArgusECSConstants::k_defaultCollisionDetectionPredictionTime;
		}

		FVector2D velocityToBoundaryOfVO = FVector2D::ZeroVector;
		ORCALine orcaLine;
		FindORCALineAndVelocityToBoundaryPerEntity(params, velocityToBoundaryOfVO, orcaLine);

		orcaLine.m_point = sourceEntityVelocity + (velocityToBoundaryOfVO / 2.0f);
		orcaLines.push_back(orcaLine);
	}

	int failureLine = -1;
	FVector2D resultingVelocity = FVector2D::ZeroVector;
	if (!TwoDimensionalLinearProgram(orcaLines, components.m_transformComponent->m_desiredSpeedUnitsPerSecond, desiredVelocity, false, resultingVelocity, failureLine))
	{
		ThreeDimensionalLinearProgram(orcaLines, components.m_transformComponent->m_desiredSpeedUnitsPerSecond, failureLine, 0, resultingVelocity);
	}

	if (CVarShowAvoidanceDebug.GetValueOnGameThread() && worldPointer.IsValid())
	{
		FVector sourceEntityLocation3D = FVector(sourceEntityLocation, 5.0f);
		DrawDebugLine(worldPointer.Get(), sourceEntityLocation3D, sourceEntityLocation3D + FVector(resultingVelocity, 0.0f), FColor::Orange, false, -1.0f, 0, 5.0f);
	}

	components.m_transformComponent->m_avoidanceVelocity = FVector(resultingVelocity, 0.0f);
}

void AvoidanceSystems::FindORCALineAndVelocityToBoundaryPerEntity(const FindORCALineParams& params, FVector2D& velocityToBoundaryOfVO, ORCALine& orcaLine)
{
	ARGUS_TRACE(AvoidanceSystems::FindORCALineAndVelocityToBoundaryPerEntity)

	const FVector2D relativeLocation = params.m_foundEntityLocation - params.m_sourceEntityLocation;
	const FVector2D relativeVelocity = params.m_sourceEntityVelocity - params.m_foundEntityVelocity;
	const float relativeLocationDistanceSquared = relativeLocation.SquaredLength();
	const float combinedRadius = 2.0f * ArgusECSConstants::k_defaultPathFindingAgentRadius;
	const float combinedRadiusSquared = FMath::Square(combinedRadius);

	if (relativeLocationDistanceSquared > combinedRadiusSquared)
	{
		// No collision yet.
		const FVector2D cutoffCenterToRelativeVelocity = relativeVelocity - (params.m_inversePredictionTime * relativeLocation);
		const float cutoffCenterToRelativeVelocityLengthSqared = cutoffCenterToRelativeVelocity.SquaredLength();
		const float dotProduct = cutoffCenterToRelativeVelocity.Dot(relativeLocation);

		if (dotProduct > 0.0f && FMath::Square(dotProduct) > combinedRadiusSquared * cutoffCenterToRelativeVelocityLengthSqared)
		{
			const float cutoffCenterToRelativeVelocityLength = FMath::Sqrt(cutoffCenterToRelativeVelocityLengthSqared);
			const FVector2D unitCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity / cutoffCenterToRelativeVelocityLength;
			orcaLine.m_direction = FVector2D(unitCutoffCenterToRelativeVelocity.Y, -unitCutoffCenterToRelativeVelocity.X);
			velocityToBoundaryOfVO = ((combinedRadius * params.m_inversePredictionTime) - cutoffCenterToRelativeVelocityLength) * unitCutoffCenterToRelativeVelocity;
		}
		else
		{
			const float leg = FMath::Sqrt(relativeLocationDistanceSquared - combinedRadiusSquared);
			if (ArgusMath::Determinant(relativeLocation, cutoffCenterToRelativeVelocity) > 0.0f)
			{
				orcaLine.m_direction = FVector2D((relativeLocation.X * leg) - (relativeLocation.Y * combinedRadius),
					(relativeLocation.X * combinedRadius) + (relativeLocation.Y * leg));
			}
			else
			{
				orcaLine.m_direction = -FVector2D((relativeLocation.X * leg) + (relativeLocation.Y * combinedRadius),
					(-relativeLocation.X * combinedRadius) + (relativeLocation.Y * leg));
			}
			orcaLine.m_direction /= relativeLocationDistanceSquared;
			velocityToBoundaryOfVO = (relativeVelocity.Dot(orcaLine.m_direction) * orcaLine.m_direction) - relativeVelocity;
		}
	}
	else
	{
		// Collision occurred.
		const float inverseDeltaTime = 1.0f / params.m_deltaTime;

		const FVector2D cutoffCenterToRelativeVelocity = relativeVelocity - (inverseDeltaTime * relativeLocation);
		const float lengthCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity.Length();
		const FVector2D normalizedCutoffCenterToRelativeVelocity = cutoffCenterToRelativeVelocity / lengthCutoffCenterToRelativeVelocity;

		orcaLine.m_direction = FVector2D(normalizedCutoffCenterToRelativeVelocity.Y, -normalizedCutoffCenterToRelativeVelocity.X);
		velocityToBoundaryOfVO = ((combinedRadius * inverseDeltaTime) - lengthCutoffCenterToRelativeVelocity) * normalizedCutoffCenterToRelativeVelocity;
	}
}

bool AvoidanceSystems::OneDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, const int lineIndex, FVector2D& resultingVelocity)
{
	ARGUS_TRACE(AvoidanceSystems::OneDimensionalLinearProgram)

	const float dotProduct = orcaLines[lineIndex].m_point.Dot(orcaLines[lineIndex].m_direction);
	const float discriminant = FMath::Square(dotProduct) + FMath::Square(radius) - orcaLines[lineIndex].m_point.SquaredLength();

	if (discriminant < 0.0f)
	{
		false;
	}

	const float sqrtDiscriminant = FMath::Sqrt(discriminant);
	float tLeft = -dotProduct - sqrtDiscriminant;
	float tRight = -dotProduct + sqrtDiscriminant;

	for (int i = 0; i < lineIndex; ++i)
	{
		const float denominator = ArgusMath::Determinant(orcaLines[lineIndex].m_direction, orcaLines[i].m_direction);
		const float numerator = ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[lineIndex].m_point - orcaLines[i].m_point);

		if (FMath::IsNearlyZero(denominator))
		{
			if (numerator < 0.0f)
			{
				return false;
			}

			continue;
		}

		const float t = numerator / denominator;

		if (denominator >= 0.0f)
		{
			tRight = FMath::Min(tRight, t);
		}
		else
		{
			tLeft = FMath::Max(tLeft, t);
		}

		if (tLeft > tRight)
		{
			return false;
		}
	}

	if (shouldOptimizeDirection)
	{
		if (preferredVelocity.Dot(orcaLines[lineIndex].m_direction) > 0.0f)
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tRight * orcaLines[lineIndex].m_direction);
		}
		else
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tLeft * orcaLines[lineIndex].m_direction);
		}
	}
	else
	{
		const float t = orcaLines[lineIndex].m_direction.Dot((preferredVelocity - orcaLines[lineIndex].m_point));
		if (t < tLeft)
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tLeft * orcaLines[lineIndex].m_direction);
		}
		else if (t < tRight)
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (tRight * orcaLines[lineIndex].m_direction);
		}
		else
		{
			resultingVelocity = orcaLines[lineIndex].m_point + (t * orcaLines[lineIndex].m_direction);
		}
	}

	return true;
}

bool AvoidanceSystems::TwoDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const FVector2D& preferredVelocity, bool shouldOptimizeDirection, FVector2D& resultingVelocity, int& failureLine)
{
	ARGUS_TRACE(AvoidanceSystems::TwoDimensionalLinearProgram)

	if (shouldOptimizeDirection)
	{
		resultingVelocity = preferredVelocity * radius;
	}
	else if (preferredVelocity.SquaredLength() > FMath::Square(radius))
	{
		resultingVelocity = preferredVelocity.GetSafeNormal() * radius;
	}
	else
	{
		resultingVelocity = preferredVelocity;
	}

	for (int i = 0; i < orcaLines.size(); ++i)
	{
		if (ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[i].m_point - resultingVelocity) > 0.0f)
		{
			const FVector2D cachedResultingVelocity = resultingVelocity;
			if (!OneDimensionalLinearProgram(orcaLines, radius, preferredVelocity, shouldOptimizeDirection, i, resultingVelocity))
			{
				resultingVelocity = resultingVelocity;
				failureLine = i;
				return false;
			}
		}
	}

	failureLine = -1;
	return true;
}

void AvoidanceSystems::ThreeDimensionalLinearProgram(const std::vector<ORCALine>& orcaLines, const float radius, const int lineIndex, const int numStaticObstacleORCALines, FVector2D& resultingVelocity)
{
	float distance = 0.0f;

	for (int i = lineIndex; i < orcaLines.size(); ++i)
	{
		if (ArgusMath::Determinant(orcaLines[i].m_direction, (orcaLines[i].m_point - resultingVelocity)) <= distance)
		{
			continue;
		}

		std::vector<ORCALine> projectedLines(orcaLines.begin(), orcaLines.begin() + static_cast<std::ptrdiff_t>(numStaticObstacleORCALines));

		for (int j = numStaticObstacleORCALines; j < i; ++j)
		{
			ORCALine orcaLine;
			const float determinant = ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[j].m_direction);
			if (FMath::IsNearlyZero(determinant))
			{
				if (orcaLines[j].m_direction.Dot(orcaLines[j].m_direction) > 0.0f)
				{
					continue;
				}

				orcaLine.m_point = 0.5f * (orcaLines[i].m_point + orcaLines[j].m_point);
			}
			else
			{
				orcaLine.m_point = orcaLines[i].m_point + ((ArgusMath::Determinant(orcaLines[j].m_direction, orcaLines[i].m_point - orcaLines[j].m_point) / determinant) * orcaLines[i].m_direction);
			}

			orcaLine.m_direction = (orcaLines[j].m_direction - orcaLines[i].m_direction).GetSafeNormal();
			projectedLines.push_back(orcaLine);
		}

		const FVector2D cachedResultingVelocity = resultingVelocity;
		int failureLine = -1;
		if (!TwoDimensionalLinearProgram(projectedLines, radius, FVector2D(-orcaLines[i].m_direction.Y, orcaLines[i].m_direction.X), true, resultingVelocity, failureLine))
		{
			resultingVelocity = cachedResultingVelocity;
		}

		distance = ArgusMath::Determinant(orcaLines[i].m_direction, orcaLines[i].m_point - resultingVelocity);
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

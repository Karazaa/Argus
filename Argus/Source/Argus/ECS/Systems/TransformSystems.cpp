// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"
#include "ArgusSystemsManager.h"
#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"

bool TransformSystems::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(TransformSystems::RunSystems);

	bool didMovementUpdateThisFrame = false;
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
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
		if (!components.m_entity || !components.m_taskComponent || !components.m_transformComponent || 
			!components.m_navigationComponent || !components.m_targetingComponent)
		{
			continue;
		}

		didMovementUpdateThisFrame |= ProcessMovementTaskCommands(worldPointer, deltaTime, components);
	}

	return didMovementUpdateThisFrame;
}

bool TransformSystems::TransformSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_navigationComponent && m_transformComponent && m_targetingComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(TransformSystemsComponentArgs));

	return false;
}

void TransformSystems::GetPathingLocationAtTimeOffset(float timeOffsetSeconds, const TransformSystemsComponentArgs& components, GetPathingLocationAtTimeOffsetResults& results)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
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

	if (numNavigationPoints == 0u || pointIndex >= numNavigationPoints - 1u)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
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

	float translationMagnitude = FMath::Abs(components.m_transformComponent->m_desiredSpeedUnitsPerSecond * timeOffsetSeconds);
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
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in an invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TransformComponent*));
		return;
	}

	vectorFromTransformToTarget.Z = 0.0f;
	if (vectorFromTransformToTarget.GetSafeNormal().Equals(transformComponent->m_transform.GetRotation().GetForwardVector()))
	{
		return;
	}

	transformComponent->m_transform.SetRotation(FRotationMatrix::MakeFromXZ(vectorFromTransformToTarget, FVector::UpVector).ToQuat());
}

void TransformSystems::MoveAlongNavigationPath(UWorld* worldPointer, float deltaTime, const TransformSystemsComponentArgs& components)
{
	ARGUS_TRACE(TransformSystems::MoveAlongNavigationPath);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	components.m_transformComponent->m_currentVelocity = components.m_transformComponent->m_proposedAvoidanceVelocity;
	const uint16 lastPointIndex = components.m_navigationComponent->m_lastPointIndex;
	const uint16 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.size();

	if (numNavigationPoints == 0u || lastPointIndex >= numNavigationPoints - 1u)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
		return;
	}

	FVector moverLocation = components.m_transformComponent->m_transform.GetLocation();
	const FVector targetLocation = components.m_navigationComponent->m_navigationPoints[lastPointIndex + 1u];
	const FVector velocity = components.m_transformComponent->m_currentVelocity * deltaTime;
	const FVector2D moverLocation2D = FVector2D(moverLocation);
	const FVector2D targetLocation2D = FVector2D(targetLocation);
	const FVector2D velocity2D = FVector2D(velocity);

	const float distanceToTarget = FVector2D::Distance(moverLocation2D, targetLocation2D);
	const float distanceVelocityUpdateToTarget = FVector2D::Distance(moverLocation2D + velocity2D, targetLocation2D);
	const float velocityLength = velocity2D.Length();
	if (FMath::IsNearlyEqual(velocityLength, distanceToTarget + distanceVelocityUpdateToTarget, ArgusECSConstants::k_moveAlongPathWaypointTolerance))
	{
		components.m_navigationComponent->m_lastPointIndex++;
		moverLocation = targetLocation;
	}
	else
	{
		moverLocation += velocity;
	}

	const bool isAtEndOfNavigationPath		=	components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 1u;
	const bool isWithinRangeOfTargetEntity	=	components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 2u &&
												components.m_taskComponent->m_movementState == MovementState::MoveToEntity &&
												components.m_targetingComponent->m_targetingRange > distanceToTarget;

	if (isWithinRangeOfTargetEntity && !isAtEndOfNavigationPath)
	{
		FaceTowardsLocationXY(components.m_transformComponent, components.m_navigationComponent->m_navigationPoints[components.m_navigationComponent->m_lastPointIndex + 1u] - moverLocation);
	}
	else
	{
		FaceTowardsLocationXY(components.m_transformComponent, components.m_transformComponent->m_currentVelocity);
	}

	moverLocation = ProjectLocationOntoNavigationData(worldPointer, components.m_transformComponent, moverLocation);
	components.m_transformComponent->m_transform.SetLocation(moverLocation);
	
	if (isAtEndOfNavigationPath || isWithinRangeOfTargetEntity)
	{
		OnCompleteNavigationPath(components);
	}
}

void TransformSystems::FindEntitiesWithinXYBounds(FVector2D minXY, FVector2D maxXY, TArray<ArgusEntity>& outEntitiesWithinBounds)
{
	ARGUS_TRACE(TransformSystems::FindEntitiesWithinXYBounds);

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

bool TransformSystems::ProcessMovementTaskCommands(UWorld* worldPointer, float deltaTime, const TransformSystemsComponentArgs& components)
{
	ARGUS_TRACE(TransformSystems::ProcessMovementTaskCommands);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	switch (components.m_taskComponent->m_movementState)
	{
		case MovementState::MoveToLocation:
		case MovementState::MoveToEntity:
			MoveAlongNavigationPath(worldPointer, deltaTime, components);
			return true;

		case MovementState::None:
			components.m_transformComponent->m_currentVelocity = components.m_transformComponent->m_proposedAvoidanceVelocity;
			if (!components.m_transformComponent->m_currentVelocity.IsNearlyZero())
			{
				const FVector velocity = components.m_transformComponent->m_currentVelocity * deltaTime;
				FaceTowardsLocationXY(components.m_transformComponent, components.m_transformComponent->m_currentVelocity);
				components.m_transformComponent->m_transform.SetLocation(components.m_transformComponent->m_transform.GetLocation() + velocity);
				return true;
			}
			return false;

		default:
			return false;
	}
}

void TransformSystems::OnCompleteNavigationPath(const TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_navigationComponent->m_queuedWaypoints.size() == 0u)
	{
		components.m_taskComponent->m_movementState = MovementState::None;
		components.m_navigationComponent->ResetPath();
		components.m_transformComponent->m_currentVelocity = FVector::ZeroVector;
	}
	else
	{
		components.m_taskComponent->m_movementState = MovementState::ProcessMoveToLocationCommand;
		components.m_navigationComponent->ResetPath();
		components.m_targetingComponent->m_targetLocation = components.m_navigationComponent->m_queuedWaypoints.front();
		components.m_navigationComponent->m_queuedWaypoints.pop();
	}
}

FVector TransformSystems::ProjectLocationOntoNavigationData(UWorld* worldPointer, TransformComponent* transformComponent, const FVector& location)
{
	if (!worldPointer)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in %s was invalid."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*))
		return location;
	}

	if (!transformComponent)
	{
		return location;
	}

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer);
	if (!unrealNavigationSystem)
	{
		return location;
	}

	FNavLocation projectedLocation;
	const FVector agentExtents = FVector(transformComponent->m_radius, transformComponent->m_radius, transformComponent->m_height / 2.0f);
	if (unrealNavigationSystem->ProjectPointToNavigation(location, projectedLocation, agentExtents, unrealNavigationSystem->MainNavData))
	{
		return projectedLocation.Location;
	}
	else
	{
		return location;
	}
}

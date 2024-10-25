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

	FVector moverLocation = components.m_transformComponent->m_transform.GetLocation();
	const FVector targetLocation = components.m_navigationComponent->m_navigationPoints[lastPointIndex + 1u];
	moverLocation += components.m_transformComponent->m_avoidanceVelocity * deltaTime;

	if (FVector::DistSquared(moverLocation, targetLocation) < 5.0f)
	{
		components.m_navigationComponent->m_lastPointIndex++;
	}

	FaceTowardsLocationXY(components.m_transformComponent, components.m_transformComponent->m_avoidanceVelocity);

	components.m_transformComponent->m_transform.SetLocation(moverLocation);
	if (components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 1u)
	{
		OnCompleteNavigationPath(components);
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
		components.m_transformComponent->m_avoidanceVelocity = FVector::ZeroVector;
	}
	else
	{
		components.m_taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
		components.m_navigationComponent->ResetPath();
		components.m_targetingComponent->m_targetLocation = components.m_navigationComponent->m_queuedWaypoints.front();
		components.m_navigationComponent->m_queuedWaypoints.pop();
	}
}
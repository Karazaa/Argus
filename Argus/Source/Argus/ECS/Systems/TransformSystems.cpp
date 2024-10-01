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
		if (!components.m_taskComponent || !components.m_transformComponent || !components.m_navigationComponent || !components.m_targetingComponent)
		{
			continue;
		}

		didMovementUpdateThisFrame |= ProcessMovementTaskCommands(deltaTime, components);
	}

	return didMovementUpdateThisFrame;
}

bool TransformSystems::TransformSystemsComponentArgs::AreComponentsValidCheck() const
{
	if (!m_entity || !m_taskComponent || !m_navigationComponent || !m_transformComponent || !m_targetingComponent)
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] Transform Systems were run with invalid entity or component arguments passed."), ARGUS_FUNCNAME);
		return false;
	}
	return true;
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
			ProcessCollisions(deltaTime, components);
			return true;
		default:
			return false;
	}
}

void TransformSystems::GetPathingLocationAtTimeOffset(float timeOffsetSeconds, const TransformSystemsComponentArgs& components, FVector& outputLocation, FVector& outputForwardVector, uint16& indexOfOutputLocation)
{
	if (!components.AreComponentsValidCheck())
	{
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
		outputLocation = currentLocation;
		outputForwardVector = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
		indexOfOutputLocation = pointIndex;
		return;
	}

	float translationMagnitude = FMath::Abs(components.m_navigationComponent->m_navigationSpeedUnitsPerSecond * timeOffsetSeconds);
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
				outputLocation = components.m_navigationComponent->m_navigationPoints[pointIndex];
				indexOfOutputLocation = pointIndex;
				return;
			}

			translationMagnitude -= positionDifferenceMagnitude;
			upcomingPoint = components.m_navigationComponent->m_navigationPoints[pointIndex + 1u];
			currentLocation = components.m_navigationComponent->m_navigationPoints[pointIndex];
			positionDifference = upcomingPoint - currentLocation;
			positionDifferenceMagnitude = positionDifference.Length();
			positionDifferenceNormalized = positionDifference.GetSafeNormal();
		}

		outputForwardVector = positionDifferenceNormalized;
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
				outputLocation = components.m_navigationComponent->m_navigationPoints[pointIndex];
				indexOfOutputLocation = 0u;
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

		outputForwardVector = -positionDifferenceNormalized;
	}

	outputLocation = currentLocation + (translationMagnitude * positionDifferenceNormalized);
	indexOfOutputLocation = pointIndex;
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

	FVector newLocation = components.m_transformComponent->m_transform.GetLocation();
	FVector newForwardVector = components.m_transformComponent->m_transform.GetRotation().GetForwardVector();
	uint16 pointIndex = lastPointIndex;
	GetPathingLocationAtTimeOffset(deltaTime, components, newLocation, newForwardVector, pointIndex);

	FaceTowardsLocationXY(components.m_transformComponent, newForwardVector);

	components.m_transformComponent->m_transform.SetLocation(newLocation);
	components.m_navigationComponent->m_lastPointIndex = pointIndex;
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

	const uint16 nearestOtherEntityId = spatialPartitioningComponent->m_argusKDTree.FindOtherArgusEntityIdClosestArgusEntity(components.m_entity);
	if (nearestOtherEntityId == ArgusECSConstants::k_maxEntities)
	{
		return;
	}

	ArgusEntity nearestOtherEntity = ArgusEntity::RetrieveEntity(nearestOtherEntityId);
	if (!nearestOtherEntity)
	{
		return;
	}

	const TransformComponent* const nearestOtherEntityTransformComponent = nearestOtherEntity.GetComponent<TransformComponent>();
	if (!nearestOtherEntityTransformComponent)
	{
		return;
	}

	const float distanceSquared = FVector::DistSquared(nearestOtherEntityTransformComponent->m_transform.GetLocation(), components.m_transformComponent->m_transform.GetLocation());
	if (distanceSquared < FMath::Square(ArgusECSConstants::k_defaultPathFindingAgentRadius))
	{
		// TODO JAMES: Actually do collision rerouting
		return;
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
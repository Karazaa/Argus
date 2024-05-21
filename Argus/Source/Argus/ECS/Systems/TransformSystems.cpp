// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"

void TransformSystems::RunSystems(float deltaTime)
{
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		std::optional<ArgusEntity> potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity.has_value())
		{
			continue;
		}

		TransformComponent* transformComponent = potentialEntity->GetComponent<TransformComponent>();
		if (!transformComponent)
		{
			continue;
		}

		ProcessMovementTaskCommands(deltaTime, potentialEntity.value(), transformComponent);
	}
}

void TransformSystems::ProcessMovementTaskCommands(float deltaTime, ArgusEntity movingEntity, TransformComponent* moverTransformComponent)
{
	if (!movingEntity || !moverTransformComponent)
	{
		return;
	}

	TaskComponent* taskComponent = movingEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	switch (taskComponent->m_currentTask)
	{
		case ETask::MoveToLocation:
		{
			MoveAlongNavigationPath(deltaTime, movingEntity, moverTransformComponent);
			break;
		}
		default:
			break;
	}
}

void TransformSystems::MoveAlongNavigationPath(float deltaTime, ArgusEntity movingEntity, TransformComponent* moverTransformComponent)
{
	if (!movingEntity || !moverTransformComponent)
	{
		return;
	}

	NavigationComponent* navigationComponent = movingEntity.GetComponent<NavigationComponent>();
	if (!navigationComponent)
	{
		return;
	}

	const uint16 lastPointIndex = navigationComponent->m_lastPointIndex;
	const uint16 numNavigationPoints = navigationComponent->m_navigationPoints.size();

	if (lastPointIndex >= numNavigationPoints - 1)
	{
		// TODO JAMES: Something went wrong
		return;
	}

	const FVector currentLocation = moverTransformComponent->m_transform.GetLocation();
	const FVector upcomingPoint = navigationComponent->m_navigationPoints[lastPointIndex + 1];
	const FVector positionDifference = upcomingPoint - currentLocation;
	const FVector positionDifferenceNormalized = positionDifference.GetSafeNormal();
	const FVector translationThisFrame = positionDifferenceNormalized * navigationComponent->m_navigationSpeedUnitsPerSecond * deltaTime;

	FaceEntityTowardsLocationXY(moverTransformComponent, positionDifference);

	if (translationThisFrame.SquaredLength() > positionDifference.SquaredLength())
	{
		moverTransformComponent->m_transform.SetLocation(upcomingPoint);
		navigationComponent->m_lastPointIndex++;
		if (navigationComponent->m_lastPointIndex == numNavigationPoints - 1)
		{
			if (TaskComponent* taskComponent = movingEntity.GetComponent<TaskComponent>())
			{
				taskComponent->m_currentTask = ETask::None;
			}
		}
	}
	else
	{
		moverTransformComponent->m_transform.SetLocation(currentLocation + translationThisFrame);
	}
}

void TransformSystems::FaceEntityTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget)
{
	if (!transformComponent)
	{
		return;
	}

	vectorFromTransformToTarget.Z = 0;
	if (FVector::Coincident(vectorFromTransformToTarget.GetSafeNormal(), transformComponent->m_transform.GetRotation().GetForwardVector()))
	{
		return;
	}
	
	transformComponent->m_transform.SetRotation(FRotationMatrix::MakeFromXZ(vectorFromTransformToTarget, FVector::UpVector).ToQuat());
}
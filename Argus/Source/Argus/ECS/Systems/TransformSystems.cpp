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

		TransformSystemsComponentArgs components;
		components.taskComponent = potentialEntity->GetComponent<TaskComponent>();
		components.transformComponent = potentialEntity->GetComponent<TransformComponent>();
		components.navigationComponent = potentialEntity->GetComponent<NavigationComponent>();
		if (!components.taskComponent || !components.transformComponent || !components.navigationComponent)
		{
			continue;
		}

		ProcessMovementTaskCommands(deltaTime, components);
	}
}

bool TransformSystems::TransformSystemsComponentArgs::AreComponentsValidCheck() const
{
	if (!taskComponent || !navigationComponent || !transformComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Transform Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return false;
	}
	return true;
}

void TransformSystems::ProcessMovementTaskCommands(float deltaTime, const TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	switch (components.taskComponent->m_currentTask)
	{
		case ETask::MoveToLocation:
			MoveAlongNavigationPath(deltaTime, components);
			break;
		default:
			break;
	}
}

void TransformSystems::MoveAlongNavigationPath(float deltaTime, const TransformSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	const uint16 lastPointIndex = components.navigationComponent->m_lastPointIndex;
	const uint16 numNavigationPoints = components.navigationComponent->m_navigationPoints.size();

	if (lastPointIndex >= numNavigationPoints - 1)
	{
		// TODO JAMES: Something went wrong
		return;
	}

	const FVector currentLocation = components.transformComponent->m_transform.GetLocation();
	const FVector upcomingPoint = components.navigationComponent->m_navigationPoints[lastPointIndex + 1];
	const FVector positionDifference = upcomingPoint - currentLocation;
	const FVector positionDifferenceNormalized = positionDifference.GetSafeNormal();
	const FVector translationThisFrame = positionDifferenceNormalized * components.navigationComponent->m_navigationSpeedUnitsPerSecond * deltaTime;

	FaceEntityTowardsLocationXY(components.transformComponent, positionDifference);

	if (translationThisFrame.SquaredLength() > positionDifference.SquaredLength())
	{
		components.transformComponent->m_transform.SetLocation(upcomingPoint);
		components.navigationComponent->m_lastPointIndex++;
		if (components.navigationComponent->m_lastPointIndex == numNavigationPoints - 1)
		{
			components.taskComponent->m_currentTask = ETask::None;
			components.navigationComponent->ResetPath();
		}
	}
	else
	{
		components.transformComponent->m_transform.SetLocation(currentLocation + translationThisFrame);
	}
}

void TransformSystems::FaceEntityTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget)
{
	if (!transformComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Transform Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return;
	}

	vectorFromTransformToTarget.Z = 0;
	if (FVector::Coincident(vectorFromTransformToTarget.GetSafeNormal(), transformComponent->m_transform.GetRotation().GetForwardVector()))
	{
		return;
	}
	
	transformComponent->m_transform.SetRotation(FRotationMatrix::MakeFromXZ(vectorFromTransformToTarget, FVector::UpVector).ToQuat());
}
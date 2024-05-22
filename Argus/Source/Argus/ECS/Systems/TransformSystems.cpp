// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"

void TransformSystems::RunSystems(float deltaTime)
{
	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		TransformSystemsComponentArgs components;
		components.m_taskComponent = potentialEntity.GetComponent<TaskComponent>();
		components.m_transformComponent = potentialEntity.GetComponent<TransformComponent>();
		components.m_navigationComponent = potentialEntity.GetComponent<NavigationComponent>();
		if (!components.m_taskComponent || !components.m_transformComponent || !components.m_navigationComponent)
		{
			continue;
		}

		ProcessMovementTaskCommands(deltaTime, components);
	}
}

bool TransformSystems::TransformSystemsComponentArgs::AreComponentsValidCheck() const
{
	if (!m_taskComponent || !m_navigationComponent || !m_transformComponent)
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

	switch (components.m_taskComponent->m_currentTask)
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

	const uint16 lastPointIndex = components.m_navigationComponent->m_lastPointIndex;
	const uint16 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.size();

	if (lastPointIndex >= numNavigationPoints - 1)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
		return;
	}

	const FVector currentLocation = components.m_transformComponent->m_transform.GetLocation();
	const FVector upcomingPoint = components.m_navigationComponent->m_navigationPoints[lastPointIndex + 1];
	const FVector positionDifference = upcomingPoint - currentLocation;
	const FVector positionDifferenceNormalized = positionDifference.GetSafeNormal();
	const FVector translationThisFrame = positionDifferenceNormalized * components.m_navigationComponent->m_navigationSpeedUnitsPerSecond * deltaTime;

	FaceEntityTowardsLocationXY(components.m_transformComponent, positionDifference);

	if (translationThisFrame.SquaredLength() > positionDifference.SquaredLength())
	{
		components.m_transformComponent->m_transform.SetLocation(upcomingPoint);
		components.m_navigationComponent->m_lastPointIndex++;
		if (components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 1)
		{
			components.m_taskComponent->m_currentTask = ETask::None;
			components.m_navigationComponent->ResetPath();
		}
	}
	else
	{
		components.m_transformComponent->m_transform.SetLocation(currentLocation + translationThisFrame);
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
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"
#include "Math/UnrealMathUtility.h"

void TransformSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TransformSystems::RunSystems)

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
		components.m_targetingComponent = potentialEntity.GetComponent<TargetingComponent>();
		if (!components.m_taskComponent || !components.m_transformComponent || !components.m_navigationComponent || !components.m_targetingComponent)
		{
			continue;
		}

		ProcessMovementTaskCommands(deltaTime, components);
	}
}

bool TransformSystems::TransformSystemsComponentArgs::AreComponentsValidCheck() const
{
	if (!m_taskComponent || !m_navigationComponent || !m_transformComponent || !m_targetingComponent)
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] Transform Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return false;
	}
	return true;
}

void TransformSystems::ProcessMovementTaskCommands(float deltaTime, const TransformSystemsComponentArgs& components)
{
	ARGUS_TRACE(TransformSystems::ProcessMovementTaskCommands)

	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	switch (components.m_taskComponent->m_currentTask)
	{
		case ETask::MoveToLocation:
		case ETask::MoveToEntity:
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
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
		return;
	}

	const FVector currentLocation = components.m_transformComponent->m_transform.GetLocation();
	const FVector upcomingPoint = components.m_navigationComponent->m_navigationPoints[lastPointIndex + 1];
	const FVector positionDifference = upcomingPoint - currentLocation;
	const FVector positionDifferenceNormalized = positionDifference.GetSafeNormal();
	const FVector translationThisFrame = positionDifferenceNormalized * components.m_navigationComponent->m_navigationSpeedUnitsPerSecond * deltaTime;

	FaceTowardsLocationXY(components.m_transformComponent, positionDifference);

	if (translationThisFrame.SquaredLength() < positionDifference.SquaredLength())
	{
		components.m_transformComponent->m_transform.SetLocation(currentLocation + translationThisFrame);
		return;
	}

	components.m_transformComponent->m_transform.SetLocation(upcomingPoint);
	components.m_navigationComponent->m_lastPointIndex++;
	if (components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 1)
	{
		OnCompleteNavigationPath(components);
	}
}

void TransformSystems::FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget)
{
	if (FMath::IsNearlyZero(vectorFromTransformToTarget.X + vectorFromTransformToTarget.Y))
	{
		return;
	}

	if (!transformComponent)
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] Transform Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return;
	}

	vectorFromTransformToTarget.Z = 0;
	if (vectorFromTransformToTarget.GetSafeNormal().Equals(transformComponent->m_transform.GetRotation().GetForwardVector()))
	{
		return;
	}
	
	transformComponent->m_transform.SetRotation(FRotationMatrix::MakeFromXZ(vectorFromTransformToTarget, FVector::UpVector).ToQuat());
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
	if (components.m_navigationComponent->m_queuedWaypoints.size() == 0)
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

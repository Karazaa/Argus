// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"
#include "ArgusMath.h"
#include "ArgusSystemsManager.h"
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

		if (components.m_taskComponent->m_constructionState == ConstructionState::BeingConstructed)
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

void TransformSystems::MoveAlongNavigationPath(UWorld* worldPointer, float deltaTime, const TransformSystemsComponentArgs& components)
{
	ARGUS_TRACE(TransformSystems::MoveAlongNavigationPath);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	components.m_transformComponent->m_currentVelocity = components.m_transformComponent->m_proposedAvoidanceVelocity;
	const int32 lastPointIndex = components.m_navigationComponent->m_lastPointIndex;
	const int32 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.Num();

	if (numNavigationPoints == 0 || lastPointIndex >= numNavigationPoints - 1)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
		return;
	}

	FVector moverLocation = components.m_transformComponent->m_location;
	const FVector targetLocation = components.m_navigationComponent->m_navigationPoints[lastPointIndex + 1u];
	const FVector velocity = components.m_transformComponent->m_currentVelocity * deltaTime;
	const FVector2D moverLocation2D = FVector2D(moverLocation);
	const FVector2D targetLocation2D = FVector2D(targetLocation);
	const FVector2D velocity2D = FVector2D(velocity);

	const float distanceToTarget = FVector2D::Distance(moverLocation2D, targetLocation2D);
	const float distanceVelocityUpdateToTarget = FVector2D::Distance(moverLocation2D + velocity2D, targetLocation2D);
	const float velocityLength = velocity2D.Length();
	bool isAtEndOfNavigationPath = false;
	if (FMath::IsNearlyEqual(velocityLength, distanceToTarget + distanceVelocityUpdateToTarget, ArgusECSConstants::k_moveAlongPathWaypointTolerance))
	{
		components.m_navigationComponent->m_lastPointIndex++;
		moverLocation = targetLocation;

		// Need to set velocity when starting pathing segment so that avoidance systems can properly consider desired velocity when proposing movement velocity.
		if (components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 1u)
		{
			isAtEndOfNavigationPath = true;
		}
		else
		{
			const FVector nextTargetLocation = components.m_navigationComponent->m_navigationPoints[components.m_navigationComponent->m_lastPointIndex + 1];
			components.m_transformComponent->m_currentVelocity = (nextTargetLocation - moverLocation).GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
		}
	}
	else
	{
		moverLocation += velocity;
	}

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
	components.m_transformComponent->m_location = moverLocation;
	components.m_transformComponent->m_smoothedYaw.SmoothChase(components.m_transformComponent->m_targetYaw, deltaTime);
	
	if (isAtEndOfNavigationPath || isWithinRangeOfTargetEntity)
	{
		OnCompleteNavigationPath(components, moverLocation);
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
				components.m_transformComponent->m_location = components.m_transformComponent->m_location + velocity;
				components.m_transformComponent->m_smoothedYaw.SmoothChase(components.m_transformComponent->m_targetYaw, deltaTime);
				return true;
			}
			return false;

		default:
			return false;
	}
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

	vectorFromTransformToTarget = ArgusMath::ToCartesianVector(vectorFromTransformToTarget).GetSafeNormal();
	const FVector currentDirection = ArgusMath::ToCartesianVector(ArgusMath::GetDirectionFromYaw(transformComponent->m_targetYaw));
	const FVector crossProduct = currentDirection.Cross(vectorFromTransformToTarget);
	const float angleDifference = FMath::Acos(vectorFromTransformToTarget.Dot(currentDirection));

	transformComponent->m_targetYaw += (angleDifference * FMath::Sign(crossProduct.Z));
}

void TransformSystems::OnCompleteNavigationPath(const TransformSystemsComponentArgs& components, const FVector& moverLocation)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	components.m_navigationComponent->m_endedNavigationLocation = moverLocation;

	if (components.m_navigationComponent->m_queuedWaypoints.IsEmpty())
	{
		components.m_taskComponent->m_movementState = MovementState::None;
		components.m_navigationComponent->ResetPath();
		components.m_transformComponent->m_currentVelocity = FVector::ZeroVector;
	}
	else
	{
		components.m_taskComponent->m_movementState = MovementState::ProcessMoveToLocationCommand;
		components.m_navigationComponent->ResetPath();
		components.m_targetingComponent->m_targetLocation = *components.m_navigationComponent->m_queuedWaypoints.Peek();
		components.m_navigationComponent->m_queuedWaypoints.Pop();
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

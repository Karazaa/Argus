// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TransformSystems.h"
#include "ArgusEntity.h"
#include "ArgusMath.h"
#include "ArgusSystemsManager.h"
#include "NavigationSystem.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/CarrierSystems.h"
#include "Systems/CombatSystems.h"
#include "Systems/DecalSystems.h"
#include "Systems/FlockingSystems.h"
#include "Systems/NavigationSystems.h"
#include "Systems/TargetingSystems.h"

bool TransformSystems::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(TransformSystems::RunSystems);

	bool didMovementUpdateThisFrame = false;

	ArgusEntity::IterateSystemsArgs<TransformSystemsArgs>([worldPointer, deltaTime, &didMovementUpdateThisFrame](TransformSystemsArgs& components) 
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			return;
		}

		const bool didEntityMove = ProcessMovementTaskCommands(worldPointer, deltaTime, components);
		didMovementUpdateThisFrame |= didEntityMove;

		// Carriers should update their passengers locations to match their location after doing an update.
		if (didEntityMove && components.m_entity.IsCarryingPassengers())
		{
			UpdatePassengerLocations(components);
		}

		if (components.m_entity.IsSelected() && components.m_taskComponent->m_flightState != EFlightState::Grounded)
		{
			ShowTraceForFlyingEntity(worldPointer, components);
		}
	});

	return didMovementUpdateThisFrame;
}

void TransformSystems::MoveAlongNavigationPath(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components)
{
	ARGUS_TRACE(TransformSystems::MoveAlongNavigationPath);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	components.m_velocityComponent->m_currentVelocity = components.m_velocityComponent->m_proposedAvoidanceVelocity;
	const int32 lastPointIndex = components.m_navigationComponent->m_lastPointIndex;
	const int32 numNavigationPoints = components.m_navigationComponent->m_navigationPoints.Num();

	if (numNavigationPoints == 0 || lastPointIndex >= numNavigationPoints - 1)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] %s exceeded %s putting pathfinding in an invalid state."), ARGUS_FUNCNAME, ARGUS_NAMEOF(lastPointIndex), ARGUS_NAMEOF(numNavigationPoints));
		return;
	}

	const bool isLastPoint = components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 2u;
	FVector moverLocation = components.m_transformComponent->m_location;
	const FVector velocity = FVector((components.m_velocityComponent->m_currentVelocity * deltaTime), 0.0f);
	moverLocation += velocity;

	TOptional<FVector> groupSourceLocation = AvoidanceSystems::GetAvoidanceGroupSourceLocation(components);
	if (groupSourceLocation.IsSet())
	{
		groupSourceLocation = groupSourceLocation.GetValue() + velocity;
	}
	else
	{
		groupSourceLocation = moverLocation;
	}

	const FVector evaluationPoint = isLastPoint ? groupSourceLocation.GetValue() : moverLocation;
	const FVector sourceLocation = components.m_navigationComponent->m_navigationPoints[lastPointIndex];
	FVector targetLocation = components.m_navigationComponent->m_navigationPoints[lastPointIndex + 1u];
	const FVector segment = targetLocation - sourceLocation;
	const FVector actual = evaluationPoint - sourceLocation;
	const float segmentLength = segment.Length();

	bool isAtEndOfNavigationPath = false;
	if ((actual.Dot(segment) / segmentLength) > segmentLength)
	{
		components.m_navigationComponent->m_lastPointIndex++;
		isAtEndOfNavigationPath = isLastPoint;

		// Need to set velocity when starting pathing segment so that avoidance systems can properly consider desired velocity when proposing movement velocity.
		if (!isLastPoint)
		{
			targetLocation = components.m_navigationComponent->m_navigationPoints[components.m_navigationComponent->m_lastPointIndex + 1];
			components.m_velocityComponent->m_currentVelocity = FVector2D((targetLocation - moverLocation).GetSafeNormal() * components.m_velocityComponent->m_desiredSpeedUnitsPerSecond);
		}
	}

	const FVector2D moverLocation2D = FVector2D(moverLocation);
	const FVector2D targetLocation2D = FVector2D(targetLocation);
	const float distanceToTarget = FVector2D::Distance(moverLocation2D, targetLocation2D);
	const bool isWithinRangeOfTargetEntity	=	components.m_navigationComponent->m_lastPointIndex == numNavigationPoints - 2u &&
												components.m_taskComponent->m_movementState == EMovementState::MoveToEntity &&
												GetEndMoveRange(components) > distanceToTarget;

	if (isWithinRangeOfTargetEntity && !isAtEndOfNavigationPath)
	{
		FaceTowardsLocationXY(components.m_transformComponent, components.m_navigationComponent->m_navigationPoints[components.m_navigationComponent->m_lastPointIndex + 1u] - moverLocation);
	}
	else
	{
		FaceTowardsLocationXY(components.m_transformComponent, FVector(components.m_velocityComponent->m_currentVelocity, 0.0f));
	}

	if (components.m_taskComponent->m_flightState == EFlightState::Grounded)
	{
		moverLocation = ProjectLocationOntoNavigationData(worldPointer, components.m_transformComponent, moverLocation);
	}
	components.m_transformComponent->m_location = moverLocation;
	components.m_transformComponent->m_smoothedYaw.SmoothChase(components.m_transformComponent->m_targetYaw, deltaTime);
	
	if (isWithinRangeOfTargetEntity)
	{
		OnWithinRangeOfTargetEntity(components);
	}
	else if (isAtEndOfNavigationPath)
	{
		OnCompleteNavigationPath(components, moverLocation);
	}
}

bool TransformSystems::ProcessMovementTaskCommands(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components)
{
	ARGUS_TRACE(TransformSystems::ProcessMovementTaskCommands);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	switch (components.m_taskComponent->m_flightState)
	{
		case EFlightState::ProcessTakeOffCommand:
			ProcessTakeOffCommand(worldPointer, deltaTime, components);
			return true;

		case EFlightState::ProcessLandCommand:
			ProcessLandCommand(worldPointer, deltaTime, components);
			return true;

		case EFlightState::TakingOff:
			PerformTakeOff(deltaTime, components);
			return true;

		case EFlightState::Landing:
			PerformLanding(deltaTime, components);
			return true;

		case EFlightState::Flying:
			break;

		case EFlightState::Grounded:
			break;

		default:
			break;
	}

	switch (components.m_taskComponent->m_movementState)
	{
		case EMovementState::MoveToLocation:
		case EMovementState::MoveToEntity:
			MoveAlongNavigationPath(worldPointer, deltaTime, components);
			return true;

		case EMovementState::InRangeOfTargetEntity:
			FaceTargetEntity(components);
			components.m_transformComponent->m_smoothedYaw.SmoothChase(components.m_transformComponent->m_targetYaw, deltaTime);
			return false;

		case EMovementState::AwaitingFinish:
			OnCompleteNavigationPath(components, components.m_transformComponent->m_location);
			return true;

		case EMovementState::None:
			components.m_velocityComponent->m_currentVelocity = components.m_velocityComponent->m_proposedAvoidanceVelocity;
			if (!components.m_velocityComponent->m_currentVelocity.IsNearlyZero())
			{
				const FVector velocity = FVector(components.m_velocityComponent->m_currentVelocity, 0.0f);
				const FVector velocityScaled = velocity * deltaTime;
				FaceTowardsLocationXY(components.m_transformComponent, velocity);
				components.m_transformComponent->m_location = components.m_transformComponent->m_location + velocityScaled;
				components.m_transformComponent->m_smoothedYaw.SmoothChase(components.m_transformComponent->m_targetYaw, deltaTime);
				return true;
			}
			return false;

		default:
			return false;
	}
}

void TransformSystems::ProcessTakeOffCommand(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	components.m_taskComponent->Set_m_flightState(EFlightState::TakingOff);
	components.m_transformComponent->m_smoothedTransitionAltitude.Reset(components.m_transformComponent->m_location.Z);

	if (!spatialPartitioningComponent->m_flyingArgusEntityKDTree.DoesArgusEntityExistInKDTree(components.m_entity))
	{
		spatialPartitioningComponent->m_flyingArgusEntityKDTree.RequestInsertArgusEntityIntoKDTree(components.m_entity);
		spatialPartitioningComponent->m_argusEntityKDTree.RequestRemoveArgusEntityIntoKDTree(components.m_entity);
	}
}

void TransformSystems::ProcessLandCommand(UWorld* worldPointer, float deltaTime, const TransformSystemsArgs& components)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	components.m_taskComponent->Set_m_flightState(EFlightState::Landing);

	FHitResult hitResult;
	const FVector startLocation = components.m_transformComponent->m_location;
	FVector endLocation = startLocation;
	endLocation.Z = ArgusECSConstants::k_lowestPossibleAltitude;
	if (worldPointer->LineTraceSingleByChannel(hitResult, startLocation, endLocation, ECC_RETICLE))
	{
		components.m_transformComponent->m_targetTransitionAltitude = ProjectLocationOntoNavigationData(worldPointer, components.m_transformComponent, hitResult.Location).Z;
	}
	else
	{
		components.m_transformComponent->m_targetTransitionAltitude = components.m_transformComponent->m_location.Z;
	}
	components.m_transformComponent->m_smoothedTransitionAltitude.Reset(components.m_transformComponent->m_location.Z);

	if (!spatialPartitioningComponent->m_argusEntityKDTree.DoesArgusEntityExistInKDTree(components.m_entity))
	{
		spatialPartitioningComponent->m_argusEntityKDTree.RequestInsertArgusEntityIntoKDTree(components.m_entity);
		spatialPartitioningComponent->m_flyingArgusEntityKDTree.RequestRemoveArgusEntityIntoKDTree(components.m_entity);
	}
}

void TransformSystems::PerformTakeOff(float deltaTime, const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	components.m_transformComponent->m_smoothedTransitionAltitude.SmoothChase(spatialPartitioningComponent->m_flyingPlaneHeight, deltaTime);
	float newAltitude = components.m_transformComponent->m_smoothedTransitionAltitude.GetValue();

	if (FMath::IsNearlyEqual(newAltitude, spatialPartitioningComponent->m_flyingPlaneHeight, ArgusECSConstants::k_flightTransitionAltitudeThreshold))
	{
		components.m_taskComponent->Set_m_flightState(EFlightState::Flying);
		newAltitude = spatialPartitioningComponent->m_flyingPlaneHeight;
	}

	components.m_transformComponent->m_location.Z = newAltitude;
}

void TransformSystems::PerformLanding(float deltaTime, const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	components.m_transformComponent->m_smoothedTransitionAltitude.SmoothChase(components.m_transformComponent->m_targetTransitionAltitude, deltaTime);
	float newAltitude = components.m_transformComponent->m_smoothedTransitionAltitude.GetValue();

	if (FMath::IsNearlyEqual(newAltitude, components.m_transformComponent->m_targetTransitionAltitude, ArgusECSConstants::k_flightTransitionAltitudeThreshold))
	{
		components.m_taskComponent->Set_m_flightState(EFlightState::Grounded);
		newAltitude = components.m_transformComponent->m_targetTransitionAltitude;
	}

	components.m_transformComponent->m_location.Z = newAltitude;
}

void TransformSystems::FaceTargetEntity(const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		return;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	const TransformComponent* targetEntityTransformComponent = targetEntity.GetComponent<TransformComponent>();
	ARGUS_RETURN_ON_NULL(targetEntityTransformComponent, ArgusECSLog);

	FaceTowardsLocationXY(components.m_transformComponent, targetEntityTransformComponent->m_location - components.m_transformComponent->m_location);
}

void TransformSystems::FaceTowardsLocationXY(TransformComponent* transformComponent, FVector vectorFromTransformToTarget)
{
	vectorFromTransformToTarget.Z = 0.0f;
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

void TransformSystems::OnWithinRangeOfTargetEntity(const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		return;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (CarrierSystems::CanEntityCarryOtherEntity(targetEntity, components.m_entity))
	{
		PassengerComponent* passengerComponent = components.m_entity.GetComponent<PassengerComponent>();
		CarrierComponent* targetCarrierComponent = targetEntity.GetComponent<CarrierComponent>();
		ARGUS_RETURN_ON_NULL(passengerComponent, ArgusECSLog);
		ARGUS_RETURN_ON_NULL(targetCarrierComponent, ArgusECSLog);
		passengerComponent->Set_m_carrierEntityId(targetEntity.GetId());
		targetCarrierComponent->m_passengerEntityIds.Add(components.m_entity.GetId());
		components.m_targetingComponent->m_targetEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	components.m_taskComponent->m_movementState = EMovementState::InRangeOfTargetEntity;
	components.m_velocityComponent->m_currentVelocity = FVector2D::ZeroVector;
}

void TransformSystems::OnCompleteNavigationPath(const TransformSystemsArgs& components, const FVector& moverLocation)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	DecalSystems::ClearMoveToLocationDecalPerEntity(components.m_entity, false);

	if (components.m_navigationComponent->m_queuedWaypoints.IsEmpty())
	{
		components.m_taskComponent->m_movementState = EMovementState::None;
		components.m_navigationComponent->ResetPath();
		components.m_velocityComponent->m_currentVelocity = FVector2D::ZeroVector;
		FlockingSystems::ChooseFlockingRootEntityIfGroupLeader(components);
	}
	else
	{
		NavigationSystems::StartNavigatingToQueuedWaypoint(components.m_taskComponent, components.m_targetingComponent, components.m_navigationComponent);
	}
}

FVector TransformSystems::ProjectLocationOntoNavigationData(UWorld* worldPointer, const TransformComponent* transformComponent, const FVector& location)
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
	const FVector agentExtents = FVector(transformComponent->m_radius, transformComponent->m_radius, ArgusECSConstants::k_navigationAgentDefaultHeight / 2.0f);
	if (unrealNavigationSystem->ProjectPointToNavigation(location, projectedLocation, agentExtents, unrealNavigationSystem->MainNavData))
	{
		return projectedLocation.Location;
	}
	else
	{
		return location;
	}
}

float TransformSystems::GetEndMoveRange(const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return components.m_velocityComponent->m_desiredSpeedUnitsPerSecond + components.m_transformComponent->m_radius;
	}

	if (components.m_taskComponent->m_movementState != EMovementState::MoveToEntity)
	{
		return components.m_targetingComponent->m_meleeRange;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		return components.m_targetingComponent->m_meleeRange;
	}

	float combinedRadius = components.m_transformComponent->m_radius;
	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (const TransformComponent* targetTransformComponent = targetEntity.GetComponent<TransformComponent>())
	{
		combinedRadius += targetTransformComponent->m_radius;
	}
	const float rangeWithoutCombinedRadius = TargetingSystems::GetRangeToUseForOtherEntity(components.m_entity, targetEntity);
	return rangeWithoutCombinedRadius + combinedRadius;
}

void TransformSystems::UpdatePassengerLocations(const TransformSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const CarrierComponent* carrierComponent = components.m_entity.GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		return;
	}

	for (int32 i = 0; i < carrierComponent->m_passengerEntityIds.Num(); ++i)
	{
		ArgusEntity passenger = ArgusEntity::RetrieveEntity(carrierComponent->m_passengerEntityIds[i]);
		if (!passenger)
		{
			return;
		}

		TransformComponent* passengerTransformComponent = passenger.GetComponent<TransformComponent>();
		if (!passengerTransformComponent)
		{
			continue;
		}

		passengerTransformComponent->m_location = components.m_transformComponent->m_location;
		passengerTransformComponent->m_targetYaw = components.m_transformComponent->m_targetYaw;
		passengerTransformComponent->m_smoothedYaw.Reset(components.m_transformComponent->m_smoothedYaw.GetValue());
	}
}

void TransformSystems::ShowTraceForFlyingEntity(UWorld* worldPointer, const TransformSystemsArgs& components)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	FVector locationWithoutHeight = components.m_transformComponent->m_location;
	locationWithoutHeight.Z = 0.0f;
	DrawDebugLine(worldPointer, components.m_transformComponent->m_location, locationWithoutHeight, FColor::Green);
}

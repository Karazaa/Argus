// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/ResourceSystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#include "DrawDebugHelpers.h"
#endif //!UE_BUILD_SHIPPING

void NavigationSystems::RunSystems(UWorld* worldPointer)
{
	ARGUS_TRACE(NavigationSystems::RunSystems);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	ArgusEntity::IterateSystemsArgs<NavigationSystemsArgs>([worldPointer](NavigationSystemsArgs& components) 
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			return;
		}

		ProcessNavigationTaskCommands(worldPointer, components);
		RecalculateMoveToEntityPaths(worldPointer, components);

#if !UE_BUILD_SHIPPING
		DrawNavigationDebugPerEntity(worldPointer, components);
#endif //!UE_BUILD_SHIPPING
	});
}

void NavigationSystems::NavigateFromEntityToEntity(UWorld* worldPointer, ArgusEntity targetEntity, const NavigationSystemsArgs& components)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!targetEntity)
	{
		return;
	}

	const TransformComponent* targetEntityTransform = targetEntity.GetComponent<TransformComponent>();
	if (!targetEntityTransform)
	{
		return;
	}

	NavigateFromEntityToLocation(worldPointer, targetEntityTransform->m_location, components);
}

void NavigationSystems::NavigateFromEntityToLocation(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsArgs& components)
{
	ARGUS_MEMORY_TRACE(ArgusNavigationSystems);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !targetLocation.has_value())
	{
		return;
	}

	components.m_taskComponent->m_constructionState = EConstructionState::None;
	components.m_navigationComponent->ResetPath();

	if (components.m_taskComponent->m_flightState == EFlightState::Grounded)
	{
		GeneratePathPointsForGroundedEntity(worldPointer, targetLocation, components);
	}
	else
	{
		GeneratePathPointsForFlyingEntity(worldPointer, targetLocation, components);
	}

	if (components.m_navigationComponent->m_navigationPoints.Num() < 2)
	{
		components.m_velocityComponent->m_currentVelocity = FVector2D::ZeroVector;
		return;
	}

	// Need to set initial velocity when starting pathing so that avoidance systems can properly consider desired velocity when starting movement.
	FVector moverLocation = components.m_transformComponent->m_location;
	const FVector firstLocation = components.m_navigationComponent->m_navigationPoints[1];
	components.m_velocityComponent->m_currentVelocity = FVector2D((firstLocation - moverLocation).GetSafeNormal() * components.m_velocityComponent->m_desiredSpeedUnitsPerSecond);
}

void NavigationSystems::StartNavigatingToQueuedWaypoint(TaskComponent* taskComponent, TargetingComponent* targetingComponent, NavigationComponent* navigationComponent)
{
	ARGUS_RETURN_ON_NULL(taskComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(targetingComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(navigationComponent, ArgusECSLog);

	if (navigationComponent->m_queuedWaypoints.IsEmpty())
	{
		return;
	}

	taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
	navigationComponent->ResetPath();
	NavigationWaypoint& nextWaypoint = navigationComponent->m_queuedWaypoints.First();
	targetingComponent->m_targetLocation = nextWaypoint.m_location;
	targetingComponent->m_decalEntityId = nextWaypoint.m_decalEntityId;
	navigationComponent->m_queuedWaypoints.PopFirst();

	if (!navigationComponent->m_queuedWaypoints.IsEmpty())
	{
		if (ArgusEntity decalEntity = ArgusEntity::RetrieveEntity(navigationComponent->m_queuedWaypoints.First().m_decalEntityId))
		{
			if (ArgusDecalComponent* decalComponent = decalEntity.GetComponent<ArgusDecalComponent>())
			{
				decalComponent->m_connectedEntityId = nextWaypoint.m_decalEntityId;
			}
		}
	}
}

void NavigationSystems::ProcessNavigationTaskCommands(UWorld* worldPointer, const NavigationSystemsArgs& components)
{
	ARGUS_TRACE(NavigationSystems::ProcessNavigationTaskCommands);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_movementState)
	{
		case EMovementState::ProcessMoveToLocationCommand:
			components.m_taskComponent->m_movementState = EMovementState::MoveToLocation;
			NavigateFromEntityToLocation(worldPointer, components.m_targetingComponent->m_targetLocation.GetValue(), components);
			ChangeTasksOnNavigatingToLocation(components);
			break;

		case EMovementState::ProcessMoveToEntityCommand:
		{
			components.m_taskComponent->m_movementState = EMovementState::MoveToEntity;
			ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
			NavigateFromEntityToEntity(worldPointer, targetEntity, components);
			ChangeTasksOnNavigatingToEntity(targetEntity, components);
			break;
		}
		default:
			break;
	}
}

void NavigationSystems::RecalculateMoveToEntityPaths(UWorld* worldPointer, const NavigationSystemsArgs& components)
{
	ARGUS_TRACE(NavigationSystems::RecalculateMoveToEntityPaths);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_movementState != EMovementState::MoveToEntity && components.m_taskComponent->m_movementState != EMovementState::InRangeOfTargetEntity)
	{
		return;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (!targetEntity)
	{
		return;
	}

	TaskComponent* targetEntityTaskComponent = targetEntity.GetComponent<TaskComponent>();
	if (!targetEntityTaskComponent)
	{
		return;
	}

	if (!targetEntityTaskComponent->IsExecutingMoveTask())
	{
		return;
	}

	NavigateFromEntityToEntity(worldPointer, targetEntity, components);
}

void NavigationSystems::ChangeTasksOnNavigatingToEntity(ArgusEntity targetEntity, const NavigationSystemsArgs& components)
{
	if (!targetEntity || !components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (CombatSystems::CanEntityAttackOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_combatState = ECombatState::DispatchedToAttack;
	}
	else
	{
		components.m_taskComponent->m_combatState = ECombatState::None;
	}

	if (ConstructionSystems::CanEntityConstructOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_constructionState = EConstructionState::DispatchedToConstructOther;
	}
	else
	{
		components.m_taskComponent->m_constructionState = EConstructionState::None;
	}

	if (ResourceSystems::CanEntityExtractResourcesFromOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::Extracting;
	}
	else if (ResourceSystems::CanEntityDepositResourcesToOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::Depositing;
	}
	else
	{
		components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::None;
	}
}

void NavigationSystems::ChangeTasksOnNavigatingToLocation(const NavigationSystemsArgs& components)
{
	components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::None;
}

void NavigationSystems::GeneratePathPointsForGroundedEntity(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsArgs& components)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !targetLocation.has_value())
	{
		return;
	}

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer);
	ARGUS_RETURN_ON_NULL(unrealNavigationSystem, ArgusECSLog);

	FPathFindingQuery pathFindingQuery = FPathFindingQuery
	(
		nullptr,
		*(unrealNavigationSystem->MainNavData),
		components.m_transformComponent->m_location,
		targetLocation.value()
	);
	pathFindingQuery.SetNavAgentProperties(FNavAgentProperties(components.m_transformComponent->m_radius, ArgusECSConstants::k_navigationAgentDefaultHeight));
	FPathFindingResult pathFindingResult = unrealNavigationSystem->FindPathSync(pathFindingQuery);

	if (!pathFindingResult.IsSuccessful() || !pathFindingResult.Path)
	{
		components.m_taskComponent->m_movementState = EMovementState::FailedToFindPath;
		return;
	}

	TArray<FNavPathPoint>& pathPoints = pathFindingResult.Path->GetPathPoints();
	const int32 numPathPoints = pathPoints.Num();

	if (numPathPoints <= 1)
	{
		components.m_taskComponent->m_movementState = EMovementState::None;
		return;
	}

	components.m_navigationComponent->m_navigationPoints.Reserve(numPathPoints);
	for (int32 i = 0; i < numPathPoints; ++i)
	{
		components.m_navigationComponent->m_navigationPoints.Add(pathPoints[i].Location);
	}
}

void NavigationSystems::GeneratePathPointsForFlyingEntity(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsArgs& components)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !targetLocation.has_value())
	{
		return;
	}

	components.m_navigationComponent->m_navigationPoints.Reserve(2);
	components.m_navigationComponent->m_navigationPoints.Add(components.m_transformComponent->m_location);

	FVector raisedTarget = targetLocation.value();
	raisedTarget.Z = components.m_transformComponent->m_location.Z;
	components.m_navigationComponent->m_navigationPoints.Add(raisedTarget);
}

#if !UE_BUILD_SHIPPING
void NavigationSystems::DrawNavigationDebugPerEntity(const UWorld* worldPointer, const NavigationSystemsArgs& components)
{	
	ARGUS_TRACE(NavigationSystems::DrawNavigationDebugPerEntity);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !ArgusECSDebugger::ShouldShowNavigationDebugForEntity(components.m_entity.GetId()))
	{
		return;
	}

	if (!components.m_taskComponent->IsExecutingMoveTask())
	{
		return;
	}

	const int32 drawIndex = components.m_navigationComponent->m_lastPointIndex + 1;
	const int32 numPathPoints = components.m_navigationComponent->m_navigationPoints.Num();
	if (drawIndex >= numPathPoints)
	{
		return;
	}

	DrawDebugLine(worldPointer, components.m_transformComponent->m_location, components.m_navigationComponent->m_navigationPoints[drawIndex], FColor::Magenta, false, -1.0f);
	for (int32 i = drawIndex; i < numPathPoints; ++i)
	{
		DrawDebugSphere(worldPointer, components.m_navigationComponent->m_navigationPoints[i], 10.0f, 10, FColor::Magenta, false, -1.0f);
		if ((i + 1) < numPathPoints)
		{
			DrawDebugLine(worldPointer, components.m_navigationComponent->m_navigationPoints[i], components.m_navigationComponent->m_navigationPoints[i + 1], FColor::Magenta, false, -1.0f);
		}
	}
}
#endif //!UE_BUILD_SHIPPING

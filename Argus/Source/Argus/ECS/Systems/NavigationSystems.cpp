// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/ResourceSystems.h"
#include "Systems/SpatialPartitioningSystems.h"
#include "Systems/TransformSystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#include "DrawDebugHelpers.h"
#endif //!UE_BUILD_SHIPPING

void NavigationSystems::RunSystems(UWorld* worldPointer)
{
	ARGUS_TRACE(NavigationSystems::RunSystems);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	ArgusEntity::IterateSystemsArgs<NavigationSystemsArgs>([](NavigationSystemsArgs& components)
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			return;
		}

		ClearAvoidanceGroupsForUpcomingPathing(components);
	});

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
	components.m_velocityComponent->m_currentVelocity = FVector2D((firstLocation - moverLocation).GetSafeNormal() * TransformSystems::GetDesiredSpeed(components.m_taskComponent, components.m_velocityComponent));
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

void NavigationSystems::ClearAvoidanceGroupsForUpcomingPathing(const NavigationSystemsArgs& components)
{
	ARGUS_TRACE(NavigationSystems::ClearAvoidanceGroupsForUpcomingPathing);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_movementState)
	{
		case EMovementState::ProcessMoveToLocationCommand:
		case EMovementState::ProcessMoveToEntityCommand:
			if (AvoidanceGroupingComponent* avoidanceGroupingComponent = components.m_entity.GetComponent<AvoidanceGroupingComponent>())
			{
				avoidanceGroupingComponent->m_groupId = ArgusECSConstants::k_maxEntities;
				avoidanceGroupingComponent->m_groupAverageLocation = FVector::ZeroVector;
				avoidanceGroupingComponent->m_numberOfIdleEntities = 0u;
				avoidanceGroupingComponent->m_entityIdsInGroup.Reset();
			}
			break;
		default:
			break;
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
			SpatialPartitioningSystems::CalculateAdjacentEntityGroupsForEntity(components.m_entity, false);
			break;

		case EMovementState::ProcessMoveToEntityCommand:
			if (TransformSystems::IsWithinEndMoveRange(components.m_entity, components.m_taskComponent, components.m_targetingComponent, components.m_transformComponent))
			{
				components.m_taskComponent->m_movementState = EMovementState::InRangeOfTargetEntity;
				ChangeTasksOnNavigatingToEntity(ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId), components);
			}
			else
			{
				components.m_taskComponent->m_movementState = EMovementState::MoveToEntity;
				ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
				NavigateFromEntityToEntity(worldPointer, targetEntity, components);
				ChangeTasksOnNavigatingToEntity(targetEntity, components);
				SpatialPartitioningSystems::CalculateAdjacentEntityGroupsForEntity(components.m_entity, false);
			}
			break;
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

	const TArray<FNavDataConfig>& allNavAgents = unrealNavigationSystem->GetSupportedAgents();
	if (allNavAgents.IsEmpty())
	{
		return;
	}

	if (!components.m_navigationComponent->m_currentNavAgentToUse.ContainsAnyAgent())
	{
		components.m_navigationComponent->m_currentNavAgentToUse = components.m_navigationComponent->m_navAgentToUseWhenSolo;
	}

	int32 propertyIndex = 0;
	for (int32 i = 0; i < allNavAgents.Num(); ++i)
	{
		if (components.m_navigationComponent->m_currentNavAgentToUse.Contains(i))
		{
			propertyIndex = i;
			break;
		}
	}

	ANavigationData* navData = unrealNavigationSystem->GetNavDataForProps(allNavAgents[propertyIndex]);
	if (!navData)
	{
		navData = unrealNavigationSystem->MainNavData;
	}

	FPathFindingQuery pathFindingQuery = FPathFindingQuery
	(
		nullptr,
		*navData,
		components.m_transformComponent->m_location,
		targetLocation.value()
	);
	pathFindingQuery.SetNavAgentProperties(allNavAgents[propertyIndex]);
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

	const TArray<FVector, ArgusContainerAllocator<15u> >* navPathPointer = &components.m_navigationComponent->m_navigationPoints;
	if (components.m_avoidanceGroupingComponent)
	{	
		ArgusEntity groupLeader = ArgusEntity::RetrieveEntity(components.m_avoidanceGroupingComponent->m_groupId);
		if (groupLeader)
		{
			if (NavigationComponent* groupLeaderNavigationComponent = groupLeader.GetComponent<NavigationComponent>())
			{
				navPathPointer = &groupLeaderNavigationComponent->m_navigationPoints;
			}
		}
	}

	const int32 drawIndex = components.m_navigationComponent->m_lastPointIndex + 1;
	const int32 numPathPoints = navPathPointer->Num();
	if (drawIndex >= numPathPoints)
	{
		return;
	}

	DrawDebugLine(worldPointer, components.m_transformComponent->m_location, (*navPathPointer)[drawIndex], FColor::Magenta, false, -1.0f);
	for (int32 i = drawIndex; i < numPathPoints; ++i)
	{
		DrawDebugSphere(worldPointer, (*navPathPointer)[i], 10.0f, 10, FColor::Magenta, false, -1.0f);
		if ((i + 1) < numPathPoints)
		{
			DrawDebugLine(worldPointer, (*navPathPointer)[i], (*navPathPointer)[i + 1], FColor::Magenta, false, -1.0f);
		}
	}
}
#endif //!UE_BUILD_SHIPPING

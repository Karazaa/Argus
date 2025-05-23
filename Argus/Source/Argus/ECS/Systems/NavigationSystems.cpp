// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/ResourceSystems.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif //!UE_BUILD_SHIPPING

void NavigationSystems::RunSystems(UWorld* worldPointer)
{
	ARGUS_TRACE(NavigationSystems::RunSystems);

	if (!IsWorldPointerValidCheck(worldPointer, ARGUS_FUNCNAME))
	{
		return;
	}

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		NavigationSystemsComponentArgs components;
		components.m_entity = ArgusEntity::RetrieveEntity(i);
		if (!components.m_entity)
		{
			continue;
		}

		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			continue;
		}

		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_navigationComponent = components.m_entity.GetComponent<NavigationComponent>();
		components.m_targetingComponent = components.m_entity.GetComponent<TargetingComponent>();
		components.m_transformComponent = components.m_entity.GetComponent<TransformComponent>();
		components.m_velocityComponent = components.m_entity.GetComponent<VelocityComponent>();
		if (!components.m_taskComponent || !components.m_navigationComponent || !components.m_targetingComponent || !components.m_transformComponent || !components.m_velocityComponent)
		{
			continue;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			continue;
		}

		ProcessNavigationTaskCommands(worldPointer, components);
		RecalculateMoveToEntityPaths(worldPointer, components);
	}
}

bool NavigationSystems::NavigationSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_navigationComponent && m_transformComponent && m_targetingComponent && m_velocityComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(NavigationSystemsComponentArgs));

	return false;
}

void NavigationSystems::NavigateFromEntityToEntity(UWorld* worldPointer, ArgusEntity targetEntity, const NavigationSystemsComponentArgs& components)
{
	if (!IsWorldPointerValidCheck(worldPointer, ARGUS_FUNCNAME) || !components.AreComponentsValidCheck(ARGUS_FUNCNAME))
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
	ChangeTasksOnNavigatingToEntity(targetEntity, components);
}

void NavigationSystems::NavigateFromEntityToLocation(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsComponentArgs& components)
{
	ARGUS_MEMORY_TRACE(ArgusNavigationSystems);

	if (!IsWorldPointerValidCheck(worldPointer, ARGUS_FUNCNAME) || !components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !targetLocation.has_value())
	{
		return;
	}

	components.m_taskComponent->m_constructionState = EConstructionState::None;

	components.m_navigationComponent->ResetPath();

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer);
	if (!unrealNavigationSystem)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not obtain a valid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(UNavigationSystemV1));
		return;
	}

	FPathFindingQuery pathFindingQuery = FPathFindingQuery
	(
		nullptr, 
		*(unrealNavigationSystem->MainNavData), 
		components.m_transformComponent->m_location, 
		targetLocation.value()
	);
	pathFindingQuery.SetNavAgentProperties(FNavAgentProperties(components.m_transformComponent->m_radius, components.m_transformComponent->m_height));
	FPathFindingResult pathFindingResult = unrealNavigationSystem->FindPathSync(pathFindingQuery);

	if (!pathFindingResult.IsSuccessful() || !pathFindingResult.Path)
	{
		components.m_taskComponent->m_movementState = EMovementState::FailedToFindPath;
		return;
	}
	
	TArray<FNavPathPoint>& pathPoints = pathFindingResult.Path->GetPathPoints();
	const int numPathPoints = pathPoints.Num();

	if (numPathPoints <= 1u)
	{
		components.m_taskComponent->m_movementState = EMovementState::None;
		return;
	}

	components.m_navigationComponent->m_navigationPoints.Reserve(numPathPoints);
	for (int i = 0; i < numPathPoints; ++i)
	{
		components.m_navigationComponent->m_navigationPoints.Add(pathPoints[i].Location);

#if !UE_BUILD_SHIPPING
		if (!ArgusECSDebugger::ShouldShowNavigationDebugForEntity(components.m_entity.GetId()))
		{
			continue;
		}

		DrawDebugSphere(worldPointer, components.m_navigationComponent->m_navigationPoints[i], 20.0f, 20, FColor::Magenta, false, 3.0f, 0, 5.0f);
		if ((i + 1) < numPathPoints)
		{
			DrawDebugLine(worldPointer, components.m_navigationComponent->m_navigationPoints[i], pathPoints[i + 1].Location, FColor::Magenta, false, 3.0f, 0, 5.0f);
		}
#endif //!UE_BUILD_SHIPPING
	}

	// Need to set initial velocity when starting pathing so that avoidance systems can properly consider desired velocity when starting movement.
	FVector moverLocation = components.m_transformComponent->m_location;
	const FVector firstLocation = components.m_navigationComponent->m_navigationPoints[1];
	components.m_velocityComponent->m_currentVelocity = FVector2D((firstLocation - moverLocation).GetSafeNormal() * components.m_velocityComponent->m_desiredSpeedUnitsPerSecond);
}

void NavigationSystems::ProcessNavigationTaskCommands(UWorld* worldPointer, const NavigationSystemsComponentArgs& components)
{
	ARGUS_TRACE(NavigationSystems::ProcessNavigationTaskCommands);

	if (!IsWorldPointerValidCheck(worldPointer, ARGUS_FUNCNAME) || !components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_movementState)
	{
		case EMovementState::ProcessMoveToLocationCommand:
			components.m_taskComponent->m_movementState = EMovementState::MoveToLocation;
			NavigateFromEntityToLocation(worldPointer, components.m_targetingComponent->m_targetLocation.GetValue(), components);
			break;

		case EMovementState::ProcessMoveToEntityCommand:
			components.m_taskComponent->m_movementState = EMovementState::MoveToEntity;
			NavigateFromEntityToEntity(worldPointer, ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId), components);
			break;

		default:
			break;
	}
}

void NavigationSystems::RecalculateMoveToEntityPaths(UWorld* worldPointer, const NavigationSystemsComponentArgs& components)
{
	ARGUS_TRACE(NavigationSystems::RecalculateMoveToEntityPaths);

	if (!IsWorldPointerValidCheck(worldPointer, ARGUS_FUNCNAME) || !components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_movementState != EMovementState::MoveToEntity)
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

bool NavigationSystems::IsWorldPointerValidCheck(UWorld* worldPointer, const WIDECHAR* functionName)
{
	if (!worldPointer)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s, %s."),
			functionName,
			ARGUS_NAMEOF(UWorld*),
			ARGUS_NAMEOF(worldPointer)
		);
		return false;
	}

	return true;
}

void NavigationSystems::ChangeTasksOnNavigatingToEntity(ArgusEntity targetEntity, const NavigationSystemsComponentArgs& components)
{
	if (!targetEntity || !components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (CombatSystems::CanEntityAttackOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_combatState = ECombatState::ShouldAttack;
	}
	else
	{
		components.m_taskComponent->m_combatState = ECombatState::None;
	}

	if (ConstructionSystems::CanEntityConstructOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_constructionState = EConstructionState::ConstructingOther;
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

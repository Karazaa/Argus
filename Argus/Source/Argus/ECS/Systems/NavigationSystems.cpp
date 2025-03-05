// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"

static TAutoConsoleVariable<bool> CVarShowNavigationDebug(TEXT("Argus.Navigation.ShowNavigationDebug"), false, TEXT(""));

void NavigationSystems::RunSystems(UWorld* worldPointer)
{
	ARGUS_TRACE(NavigationSystems::RunSystems);

	if (!IsWorldPointerValidCheck(worldPointer, ARGUS_FUNCNAME))
	{
		return;
	}

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		NavigationSystemsComponentArgs components;
		components.m_entity = potentialEntity;
		components.m_taskComponent = potentialEntity.GetComponent<TaskComponent>();
		components.m_navigationComponent = potentialEntity.GetComponent<NavigationComponent>();
		components.m_targetingComponent = potentialEntity.GetComponent<TargetingComponent>();
		components.m_transformComponent = potentialEntity.GetComponent<TransformComponent>();
		if (!components.m_taskComponent || !components.m_navigationComponent || !components.m_targetingComponent || !components.m_transformComponent)
		{
			continue;
		}

		if (components.m_taskComponent->m_constructionState == ConstructionState::BeingConstructed)
		{
			continue;
		}

		ProcessNavigationTaskCommands(worldPointer, components);
		RecalculateMoveToEntityPaths(worldPointer, components);
	}
}

bool NavigationSystems::NavigationSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (!m_entity || !m_taskComponent || !m_navigationComponent || !m_targetingComponent || !m_transformComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Navigation Systems were run with invalid component arguments passed."), functionName);
		return false;
	}

	return true;
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

	if (CombatSystems::CanEntityAttackOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_combatState = CombatState::Attack;
	}
	if (ConstructionSystems::CanEntityConstructOtherEntity(components.m_entity, targetEntity))
	{
		components.m_taskComponent->m_constructionState = ConstructionState::ConstructingOther;
	}
}

void NavigationSystems::NavigateFromEntityToLocation(UWorld* worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsComponentArgs& components)
{
	ARGUS_MEMORY_TRACE(ArgusNavigationSystems);

	if (!IsWorldPointerValidCheck(worldPointer, ARGUS_FUNCNAME) || !components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !targetLocation.has_value())
	{
		return;
	}

	components.m_taskComponent->m_constructionState = ConstructionState::None;

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
		components.m_taskComponent->m_movementState = MovementState::FailedToFindPath;
		return;
	}
	
	TArray<FNavPathPoint>& pathPoints = pathFindingResult.Path->GetPathPoints();
	const int numPathPoints = pathPoints.Num();

	if (numPathPoints <= 1u)
	{
		components.m_taskComponent->m_movementState = MovementState::None;
		return;
	}

	components.m_navigationComponent->m_navigationPoints.Reserve(numPathPoints);
	for (int i = 0; i < numPathPoints; ++i)
	{
		components.m_navigationComponent->m_navigationPoints.Add(pathPoints[i].Location);

		if (!CVarShowNavigationDebug.GetValueOnGameThread())
		{
			continue;
		}

		DrawDebugSphere(worldPointer, components.m_navigationComponent->m_navigationPoints[i], 20.0f, 20, FColor::Magenta, false, 3.0f, 0, 5.0f);
		if ((i + 1) < numPathPoints)
		{
			DrawDebugLine(worldPointer, components.m_navigationComponent->m_navigationPoints[i], pathPoints[i + 1].Location, FColor::Magenta, false, 3.0f, 0, 5.0f);
		}
	}

	// Need to set initial velocity when starting pathing so that avoidance systems can properly consider desired velocity when starting movement.
	FVector moverLocation = components.m_transformComponent->m_location;
	const FVector firstLocation = components.m_navigationComponent->m_navigationPoints[1];
	components.m_transformComponent->m_currentVelocity = (firstLocation - moverLocation).GetSafeNormal() * components.m_transformComponent->m_desiredSpeedUnitsPerSecond;
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
		case MovementState::ProcessMoveToLocationCommand:
			components.m_taskComponent->m_movementState = MovementState::MoveToLocation;
			NavigateFromEntityToLocation(worldPointer, components.m_targetingComponent->m_targetLocation.GetValue(), components);
			break;

		case MovementState::ProcessMoveToEntityCommand:
			components.m_taskComponent->m_movementState = MovementState::MoveToEntity;
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

	if (components.m_taskComponent->m_movementState != MovementState::MoveToEntity)
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
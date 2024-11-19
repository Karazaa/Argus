// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"

static TAutoConsoleVariable<bool> CVarShowNavigationDebug(TEXT("Argus.Navigation.ShowNavigationDebug"), false, TEXT(""));

void NavigationSystems::RunSystems(TWeakObjectPtr<UWorld>& worldPointer)
{
	ARGUS_TRACE(NavigationSystems::RunSystems)

	if (!IsWorldPointerValidCheck(worldPointer))
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
		components.m_taskComponent = potentialEntity.GetComponent<TaskComponent>();
		components.m_navigationComponent = potentialEntity.GetComponent<NavigationComponent>();
		components.m_targetingComponent = potentialEntity.GetComponent<TargetingComponent>();
		components.m_transformComponent = potentialEntity.GetComponent<TransformComponent>();
		if (!components.m_taskComponent || !components.m_navigationComponent || !components.m_targetingComponent || !components.m_transformComponent)
		{
			continue;
		}

		ProcessNavigationTaskCommands(worldPointer, components);
		RecalculateMoveToEntityPaths(worldPointer, components);
	}
}

bool NavigationSystems::NavigationSystemsComponentArgs::AreComponentsValidCheck() const
{
	if (!m_taskComponent || !m_navigationComponent || !m_targetingComponent || !m_transformComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Navigation Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return false;
	}

	return true;
}

void NavigationSystems::ProcessNavigationTaskCommands(TWeakObjectPtr<UWorld>& worldPointer, const NavigationSystemsComponentArgs& components)
{
	ARGUS_TRACE(NavigationSystems::ProcessNavigationTaskCommands)

	if (!IsWorldPointerValidCheck(worldPointer) || !components.AreComponentsValidCheck())
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

void NavigationSystems::RecalculateMoveToEntityPaths(TWeakObjectPtr<UWorld>& worldPointer, const NavigationSystemsComponentArgs& components)
{
	ARGUS_TRACE(NavigationSystems::RecalculateMoveToEntityPaths)

	if (!IsWorldPointerValidCheck(worldPointer) || !components.AreComponentsValidCheck())
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

void NavigationSystems::NavigateFromEntityToEntity(TWeakObjectPtr<UWorld>& worldPointer, ArgusEntity targetEntity, const NavigationSystemsComponentArgs& components)
{
	if (!IsWorldPointerValidCheck(worldPointer) || !components.AreComponentsValidCheck())
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

	NavigateFromEntityToLocation(worldPointer, targetEntityTransform->m_transform.GetLocation(), components);
}

void NavigationSystems::NavigateFromEntityToLocation(TWeakObjectPtr<UWorld>& worldPointer, std::optional<FVector> targetLocation, const NavigationSystemsComponentArgs& components)
{
	if (!IsWorldPointerValidCheck(worldPointer) || !components.AreComponentsValidCheck() || !targetLocation.has_value())
	{
		return;
	}

	components.m_navigationComponent->ResetPath();

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer.Get());
	if (!unrealNavigationSystem)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not obtain a valid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(UNavigationSystemV1));
		return;
	}

	FPathFindingQuery pathFindingQuery = FPathFindingQuery
	(
		nullptr, 
		*(unrealNavigationSystem->MainNavData), 
		components.m_transformComponent->m_transform.GetLocation(), 
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

	components.m_navigationComponent->m_navigationPoints.reserve(numPathPoints);
	for (int i = 0; i < numPathPoints; ++i)
	{
		components.m_navigationComponent->m_navigationPoints.emplace_back(pathPoints[i].Location);

		if (!CVarShowNavigationDebug.GetValueOnGameThread())
		{
			continue;
		}

		DrawDebugSphere(worldPointer.Get(), components.m_navigationComponent->m_navigationPoints[i], 20.0f, 20, FColor::Magenta, false, 3.0f, 0, 5.0f);
		if ((i + 1) < numPathPoints)
		{
			DrawDebugLine(worldPointer.Get(), components.m_navigationComponent->m_navigationPoints[i], pathPoints[i + 1].Location, FColor::Magenta, false, 3.0f, 0, 5.0f);
		}
	}
}

bool NavigationSystems::IsWorldPointerValidCheck(TWeakObjectPtr<UWorld>& worldPointer)
{
	if (!worldPointer.IsValid())
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s, %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TWeakObjectPtr<UWorld>),
			ARGUS_NAMEOF(worldPointer)
		);
		return false;
	}

	return true;
}
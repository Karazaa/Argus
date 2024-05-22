// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"

static TAutoConsoleVariable<bool> CVarShowNavigationDebug(TEXT("Argus.Navigation.ShowNavigationDebug"), false, TEXT(""));

void NavigationSystems::RunSystems(TWeakObjectPtr<UWorld> worldPointer)
{
	if (!IsWorldPointerValidCheck(worldPointer))
	{
		return;
	}

	for (uint16 i = 0; i < ArgusECSConstants::k_maxEntities; ++i)
	{
		std::optional<ArgusEntity> potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity.has_value())
		{
			continue;
		}

		NavigationSystemsComponentArgs components;
		components.m_taskComponent = potentialEntity->GetComponent<TaskComponent>();
		components.m_navigationComponent = potentialEntity->GetComponent<NavigationComponent>();
		components.m_targetingComponent = potentialEntity->GetComponent<TargetingComponent>();
		components.m_transformComponent = potentialEntity->GetComponent<TransformComponent>();
		if (!components.m_taskComponent || !components.m_navigationComponent || !components.m_targetingComponent || !components.m_transformComponent)
		{
			continue;
		}

		ProcessNavigationTaskCommands(worldPointer, components);
	}
}

bool NavigationSystems::NavigationSystemsComponentArgs::AreComponentsValidCheck() const
{
	if (!m_taskComponent || !m_navigationComponent || !m_targetingComponent || !m_transformComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Navigation Systems were run with invalid component arguments passed."), ARGUS_FUNCNAME);
		return false;
	}

	return true;
}

void NavigationSystems::ProcessNavigationTaskCommands(TWeakObjectPtr<UWorld> worldPointer, const NavigationSystemsComponentArgs& components)
{
	if (!IsWorldPointerValidCheck(worldPointer) || !components.AreComponentsValidCheck())
	{
		return;
	}

	switch (components.m_taskComponent->m_currentTask)
	{
		case ETask::ProcessMoveToLocationCommand:
		{
			components.m_taskComponent->m_currentTask = ETask::MoveToLocation;
			
			if (!components.m_targetingComponent->HasLocationTarget())
			{
				return;
			}
			NavigateFromEntityToLocation(worldPointer, components.m_targetingComponent->m_targetLocation.GetValue(), components);
			break;
		}
		case ETask::ProcessMoveToEntityCommand:
			// TODO JAMES: Generate nav path to entity.
			components.m_taskComponent->m_currentTask = ETask::MoveToEntity;
			break;
		default:
			break;
	}
}

void NavigationSystems::NavigateFromEntityToLocation(TWeakObjectPtr<UWorld> worldPointer, FVector targetLocation, const NavigationSystemsComponentArgs& components)
{
	if (!IsWorldPointerValidCheck(worldPointer) || !components.AreComponentsValidCheck())
	{
		return;
	}

	components.m_navigationComponent->ResetPath();

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer.Get());
	if (!unrealNavigationSystem)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Could not obtain a valid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(UNavigationSystemV1));
		return;
	}

	FPathFindingQuery pathFindingQuery = FPathFindingQuery(nullptr, *(unrealNavigationSystem->MainNavData), components.m_transformComponent->m_transform.GetLocation(), targetLocation);
	pathFindingQuery.SetNavAgentProperties(FNavAgentProperties(ArgusECSConstants::k_defaultPathFindingAgentRadius, ArgusECSConstants::k_defaultPathFindingAgentHeight));
	FPathFindingResult pathFindingResult = unrealNavigationSystem->FindPathSync(pathFindingQuery);

	if (!pathFindingResult.IsSuccessful() || !pathFindingResult.Path)
	{
		components.m_taskComponent->m_currentTask = ETask::FailedToFindPath;
		return;
	}

	TArray<FNavPathPoint>& pathPoints = pathFindingResult.Path->GetPathPoints();
	int numPathPoints = pathPoints.Num();
	components.m_navigationComponent->m_navigationPoints.reserve(numPathPoints);

	for (int i = 0; i < numPathPoints; ++i)
	{
		components.m_navigationComponent->m_navigationPoints.emplace_back(pathPoints[i].Location);

		if (CVarShowNavigationDebug.GetValueOnGameThread())
		{
			DrawDebugSphere(worldPointer.Get(), components.m_navigationComponent->m_navigationPoints[i], 20.0f, 20, FColor::Magenta, false, 3.0f, 0, 5.0f);
			if ((i + 1) < numPathPoints)
			{
				DrawDebugLine(worldPointer.Get(), components.m_navigationComponent->m_navigationPoints[i], pathPoints[i + 1].Location, FColor::Magenta, false, 3.0f, 0, 5.0f);
			}
		}
	}
}

bool NavigationSystems::IsWorldPointerValidCheck(TWeakObjectPtr<UWorld> worldPointer)
{
	if (!worldPointer.IsValid())
	{
		UE_LOG
		(
			ArgusGameLog, Error, TEXT("[%s] was invoked with an invalid %s, %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TWeakObjectPtr<UWorld>),
			ARGUS_NAMEOF(worldPointer)
		);
		return false;
	}

	return true;
}
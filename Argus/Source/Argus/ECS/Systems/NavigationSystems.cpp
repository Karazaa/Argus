// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
#include "DrawDebugHelpers.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "NavigationSystemTypes.h"

void NavigationSystems::RunSystems(TWeakObjectPtr<UWorld> worldPointer, float deltaTime)
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

		NavigationComponent* navigationComponent = potentialEntity->GetComponent<NavigationComponent>();
		if (!navigationComponent)
		{
			continue;
		}

		ProcessNavigationTaskCommands(worldPointer, potentialEntity.value(), navigationComponent);
	}
}

void NavigationSystems::ProcessNavigationTaskCommands(TWeakObjectPtr<UWorld> worldPointer, ArgusEntity sourceEntity, NavigationComponent* sourceNavigationComponent)
{
	if (!IsWorldPointerValidCheck(worldPointer) || !IsSourceEntityValidCheck(sourceEntity) || !IsSourceNavigationComponentValidCheck(sourceNavigationComponent))
	{
		return;
	}

	TaskComponent* taskComponent = sourceEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	switch (taskComponent->m_currentTask)
	{
		case ETask::ProcessMoveToLocationCommand:
		{
			taskComponent->m_currentTask = ETask::MoveToLocation;
			TargetingComponent* targetingComponent = sourceEntity.GetComponent<TargetingComponent>();
			if (!targetingComponent || !targetingComponent->HasLocationTarget())
			{
				return;
			}
			NavigateFromEntityToLocation(worldPointer, sourceEntity, sourceNavigationComponent, targetingComponent->m_targetLocation.GetValue());
			break;
		}
		case ETask::ProcessMoveToEntityCommand:
			// TODO JAMES: Generate nav path to entity.
			taskComponent->m_currentTask = ETask::MoveToEntity;
			break;
		default:
			break;
	}
}

void NavigationSystems::NavigateFromEntityToLocation(TWeakObjectPtr<UWorld> worldPointer, ArgusEntity sourceEntity, NavigationComponent* sourceNavigationComponent, FVector targetLocation)
{
	if (!IsWorldPointerValidCheck(worldPointer) || !IsSourceEntityValidCheck(sourceEntity) || !IsSourceNavigationComponentValidCheck(sourceNavigationComponent))
	{
		return;
	}

	sourceNavigationComponent->ResetPath();

	TransformComponent* transformComponent = sourceEntity.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer.Get());
	if (!unrealNavigationSystem)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Could not obtain a valid reference to %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(UNavigationSystemV1));
		return;
	}

	FPathFindingQuery pathFindingQuery = FPathFindingQuery(nullptr, *(unrealNavigationSystem->MainNavData), transformComponent->m_transform.GetLocation(), targetLocation);
	pathFindingQuery.SetNavAgentProperties(FNavAgentProperties(ArgusECSConstants::k_defaultPathFindingAgentRadius, ArgusECSConstants::k_defaultPathFindingAgentHeight));
	FPathFindingResult pathFindingResult = unrealNavigationSystem->FindPathSync(pathFindingQuery);

	if (!pathFindingResult.IsSuccessful() || !pathFindingResult.Path)
	{
		TaskComponent* taskComponent = sourceEntity.GetComponent<TaskComponent>();
		if (taskComponent)
		{
			taskComponent->m_currentTask = ETask::FailedToFindPath;
		}
		return;
	}

	TArray<FNavPathPoint>& pathPoints = pathFindingResult.Path->GetPathPoints();
	int numPathPoints = pathPoints.Num();
	sourceNavigationComponent->m_navigationPoints.reserve(numPathPoints);

	for (int i = 0; i < numPathPoints; ++i)
	{
		sourceNavigationComponent->m_navigationPoints.emplace_back(pathPoints[i].Location);

		DrawDebugSphere(worldPointer.Get(), sourceNavigationComponent->m_navigationPoints[i], 20.0f, 20, FColor::Magenta, false, 3.0f, 0, 5.0f);
		if ((i + 1) < numPathPoints)
		{
			DrawDebugLine(worldPointer.Get(), sourceNavigationComponent->m_navigationPoints[i], pathPoints[i + 1].Location, FColor::Magenta, false, 3.0f, 0, 5.0f);
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

bool NavigationSystems::IsSourceEntityValidCheck(ArgusEntity sourceEntity)
{
	if (!sourceEntity)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid %s passed for %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(sourceEntity));
		return false;
	}
	return true;
}

bool NavigationSystems::IsSourceNavigationComponentValidCheck(NavigationComponent* sourceNavigationComponent)
{
	if (!sourceNavigationComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null %s passed for source entity."), ARGUS_FUNCNAME, ARGUS_NAMEOF(NavigationComponent));
		return false;
	}
	return true;
}
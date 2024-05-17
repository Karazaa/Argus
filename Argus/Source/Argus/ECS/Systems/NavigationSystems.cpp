// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"
#include "AI/Navigation/NavigationTypes.h"
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

	TransformComponent* transformComponent = sourceEntity.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(worldPointer.Get());
	if (!unrealNavigationSystem)
	{
		// TODO JAMES: Error here
		return;
	}

	FPathFindingQuery pathFindingQuery = FPathFindingQuery(nullptr, *(unrealNavigationSystem->MainNavData), transformComponent->m_transform.GetLocation(), targetLocation);
	pathFindingQuery.SetNavAgentProperties(FNavAgentProperties(ArgusECSConstants::k_defaultPathFindingAgentRadius, ArgusECSConstants::k_defaultPathFindingAgentHeight));
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
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationSystems.h"

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
	if (!IsWorldPointerValidCheck(worldPointer))
	{
		return;
	}

	if (!sourceEntity)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Invalid %s passed for %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(sourceEntity));
		return;
	}

	if (!sourceNavigationComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null %s passed for entity %d."), ARGUS_FUNCNAME, ARGUS_NAMEOF(NavigationComponent), sourceEntity.GetId());
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
			// TODO JAMES: Generate nav path to location
			taskComponent->m_currentTask = ETask::MoveToLocation;
			break;
		case ETask::ProcessMoveToEntityCommand:
			// TODO JAMES: Generate nav path to entity.
			taskComponent->m_currentTask = ETask::MoveToEntity;
			break;
		default:
			break;
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
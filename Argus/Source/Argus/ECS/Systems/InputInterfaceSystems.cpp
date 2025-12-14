// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "InputInterfaceSystems.h"
#include "ArgusGameInstance.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void InputInterfaceSystems::CheckAndHandleEntityDoubleClick(ArgusEntity entity)
{
	if (!entity)
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	if (inputInterfaceComponent->m_doubleClickTimer.IsTimerTicking(singletonEntity))
	{
		if (entity.GetId() == inputInterfaceComponent->m_lastSelectedEntityId)
		{
			inputInterfaceComponent->m_lastSelectedEntityId = ArgusECSConstants::k_maxEntities;
			inputInterfaceComponent->m_doubleClickTimer.CancelTimer(singletonEntity);

			AddAdjacentLikeEntitiesAsSelected(entity, inputInterfaceComponent);
			return;
		}
	}

	inputInterfaceComponent->m_lastSelectedEntityId = entity.GetId();
	inputInterfaceComponent->m_doubleClickTimer.CancelTimer(singletonEntity);
	inputInterfaceComponent->m_doubleClickTimer.StartTimer(singletonEntity, inputInterfaceComponent->m_doubleClickThresholdSeconds);
}

void InputInterfaceSystems::AddAdjacentLikeEntitiesAsSelected(ArgusEntity entity, InputInterfaceComponent* inputInterfaceComponent)
{
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	if (!taskComponent && !transformComponent)
	{
		return;
	}

	TFunction<bool(const ArgusEntityKDTreeNode*)> predicate = [entity, taskComponent, inputInterfaceComponent](const ArgusEntityKDTreeNode* node)
	{
		ARGUS_RETURN_ON_NULL_BOOL(taskComponent, ArgusECSLog);
		ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusECSLog);
		if (!node)
		{
			return false;
		}

		ArgusEntity foundEntity = ArgusEntity::RetrieveEntity(node->m_entityId);
		if (!foundEntity || inputInterfaceComponent->IsEntityIdSelected(node->m_entityId))
		{
			return false;
		}

		const TaskComponent* foundTaskComponent = foundEntity.GetComponent<TaskComponent>();
		if (!foundTaskComponent)
		{
			return false;
		}

		if (!foundEntity.IsOnSameTeamAsOtherEntity(entity))
		{
			return false;
		}

		return foundTaskComponent->m_spawnedFromArgusActorRecordId == taskComponent->m_spawnedFromArgusActorRecordId;
	};

	TArray<uint16> foundFlyingEntityIds;
	if (transformComponent->CanFly())
	{
		spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(foundFlyingEntityIds, transformComponent->m_location, inputInterfaceComponent->m_doubleClickQueryRange, predicate);
	}

	TArray<uint16> foundGroundedEntityIds;
	if (transformComponent->CanWalk())
	{
		spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(foundGroundedEntityIds, transformComponent->m_location, inputInterfaceComponent->m_doubleClickQueryRange, predicate);
	}

	for (int32 i = 0u; i < foundFlyingEntityIds.Num(); ++i)
	{
		AddEntityIdAsSelected(foundFlyingEntityIds[i]);
	}

	for (int32 i = 0u; i < foundGroundedEntityIds.Num(); ++i)
	{
		AddEntityIdAsSelected(foundGroundedEntityIds[i]);
	}
}

void InputInterfaceSystems::AddEntityIdAsSelected(uint16 entityId)
{
	UArgusGameInstance* gameInstance = UArgusGameInstance::GetArgusGameInstance();
	ARGUS_RETURN_ON_NULL(gameInstance, ArgusECSLog);

	AArgusActor* actor = gameInstance->GetArgusActorFromArgusEntity(ArgusEntity::RetrieveEntity(entityId));
	ARGUS_RETURN_ON_NULL(actor, ArgusECSLog);

	// TODO JAMES: Set selected state and place into interface arrays for selected entities and/or active ability groups.
}

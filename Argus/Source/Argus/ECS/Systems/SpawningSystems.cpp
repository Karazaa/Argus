// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpawningSystems.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"
#include "HAL/UnrealMemory.h"
#include "Systems/ConstructionSystems.h"

bool SpawningSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(SpawningSystems::RunSystems);

	bool spawnedAnEntityThisFrame = false;

	ArgusEntity::IterateSystemsArgs<SpawningSystemsArgs>([deltaTime, &spawnedAnEntityThisFrame](SpawningSystemsArgs& components)
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		if (components.m_taskComponent->m_constructionState == EConstructionState::BeingConstructed)
		{
			return;
		}

		if (components.m_spawningComponent->m_spawnQueueIndexToCancel.IsSet())
		{
			ProcessCancelationRequest(components);
		}

		spawnedAnEntityThisFrame |= ProcessSpawningTaskCommands(deltaTime, components);
	});

	return spawnedAnEntityThisFrame;
}

void SpawningSystems::SpawnEntity(const SpawningSystemsArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord)
{
	ARGUS_TRACE(SpawningSystems::SpawnEntity);
	SpawnEntityInternal(components, spawnInfo, overrideArgusActorRecord);
	ProcessQueuedSpawnEntity(components);
}

void SpawningSystems::SpawnEntityInternal(const SpawningSystemsArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord)
{
	ARGUS_TRACE(SpawningSystems::SpawnEntityInternal);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const UArgusActorRecord* argusActorRecord = overrideArgusActorRecord ? overrideArgusActorRecord : ArgusStaticData::GetRecord<UArgusActorRecord>(spawnInfo.m_argusActorRecordId);
	ARGUS_RETURN_ON_NULL(argusActorRecord, ArgusECSLog);
	const UArgusEntityTemplate* argusEntityTemplate = argusActorRecord->m_entityTemplate.LoadAndStorePtr();
	ARGUS_RETURN_ON_NULL(argusEntityTemplate, ArgusECSLog);

	if (components.m_taskComponent->m_spawningState != ESpawningState::SpawningEntity)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not spawn %s. %s is not equal to %s."), 
			ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(m_spawningState), ARGUS_NAMEOF(ESpawningState::SpawningEntity)
		);
		return;
	}

	components.m_taskComponent->m_spawningState = ESpawningState::None;

	ArgusEntity spawnedEntity = argusEntityTemplate->MakeEntity();
	TaskComponent* spawnedEntityTaskComponent = spawnedEntity.GetOrAddComponent<TaskComponent>();
	if (!spawnedEntityTaskComponent)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not retrieve a %s from the spawned %s."), 
			ARGUS_FUNCNAME, ARGUS_NAMEOF(TaskComponent), ARGUS_NAMEOF(ArgusEntity)
		);
		return;
	}

	spawnedEntityTaskComponent->m_baseState = EBaseState::SpawnedWaitingForActorTake;
	spawnedEntityTaskComponent->m_spawnedFromArgusActorRecordId = argusActorRecord->m_id;

	if (IdentityComponent* spawningEntityIdentityComponent = components.m_entity.GetComponent<IdentityComponent>())
	{
		if (IdentityComponent* spawnedEntityIdentityComponent = spawnedEntity.GetComponent<IdentityComponent>())
		{
			*spawnedEntityIdentityComponent = *spawningEntityIdentityComponent;
		}
	}

	TransformComponent* spawnedEntityTransformComponent = spawnedEntity.GetComponent<TransformComponent>();
	if (!spawnedEntityTransformComponent)
	{
		return;
	}

	FVector spawnLocation = components.m_transformComponent->m_location;
	EMovementState initialSpawnMovementState = EMovementState::None;

	if (spawnInfo.m_spawnLocationOverride.IsSet())
	{
		spawnLocation = spawnInfo.m_spawnLocationOverride.GetValue();
	}
	else
	{
		GetSpawnLocationAndNavigationState(components, spawnLocation, initialSpawnMovementState);
	}

	spawnedEntityTransformComponent->m_location = spawnLocation;

	if (spawnInfo.m_needsConstruction)
	{
		spawnedEntityTaskComponent->m_constructionState = EConstructionState::BeingConstructed;
		if (ConstructionComponent* constructionComponent = spawnedEntity.GetOrAddComponent<ConstructionComponent>())
		{
			constructionComponent->m_constructionAbilityRecordId = spawnInfo.m_spawningAbilityRecordId;

			if (constructionComponent->m_constructionType == EConstructionType::Automatic)
			{
				constructionComponent->m_automaticConstructionTimerHandle.StartTimer(spawnedEntity, constructionComponent->m_requiredWorkSeconds);
			}
			else if (constructionComponent->m_constructionType == EConstructionType::Manual)
			{
				CommandMoveSelectedEntitiesToSpawnedEntity(spawnedEntity, true);
			}
		}
	}

	if (ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity())
	{
		if (SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>())
		{
			if (spawnedEntityTaskComponent->m_flightState == EFlightState::Flying || spawnedEntityTaskComponent->m_flightState == EFlightState::Landing)
			{
				spawnedEntityTransformComponent->m_location.Z = spatialPartitioningComponent->m_flyingPlaneHeight;
				spatialPartitioningComponent->m_flyingArgusEntityKDTree.InsertArgusEntityIntoKDTree(spawnedEntity);
			}
			else
			{
				spatialPartitioningComponent->m_argusEntityKDTree.InsertArgusEntityIntoKDTree(spawnedEntity);
			}
		}
	}

	NavigationComponent* spawnedEntityNavigationComponent = spawnedEntity.GetComponent<NavigationComponent>();
	TargetingComponent* spawnedEntityTargetingComponent = spawnedEntity.GetComponent<TargetingComponent>();
	if (!spawnedEntityNavigationComponent || !spawnedEntityTargetingComponent)
	{
		return;
	}

	spawnedEntityTaskComponent->m_movementState = initialSpawnMovementState;
	*spawnedEntityTargetingComponent = *components.m_targetingComponent;
}

void SpawningSystems::SpawnEntityFromQueue(const SpawningSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_spawningState != ESpawningState::SpawningEntity)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] The %s of the spawning %s is not %s"), 
			ARGUS_FUNCNAME, ARGUS_NAMEOF(m_spawningState), ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ESpawningState::SpawningEntity)
		);
		return;
	}

	SpawnEntityInfo spawnInfo;
	if (!components.m_spawningComponent->m_spawnQueue.TryPopFirst(spawnInfo))
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not dequeue a %s from the spawner's %s."),
			ARGUS_FUNCNAME, ARGUS_NAMEOF(SpawnEntityInfo), ARGUS_NAMEOF(SpawningComponent)
		);
		return;
	}

	SpawnEntity(components, spawnInfo);
}

void SpawningSystems::ProcessCancelationRequest(const SpawningSystemsArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessCancelationRequest);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const uint8 indexToCancel = components.m_spawningComponent->m_spawnQueueIndexToCancel.GetValue();
	components.m_spawningComponent->m_spawnQueueIndexToCancel.Reset();

	for (uint8 i = indexToCancel; i < components.m_spawningComponent->m_spawnQueue.Num() - 1; ++i)
	{
		components.m_spawningComponent->m_spawnQueue[i] = components.m_spawningComponent->m_spawnQueue[i + 1];
	}

	if (components.m_spawningComponent->m_spawnQueue[indexToCancel].m_spawningAbilityRecordId > 0u)
	{
		if (AbilityComponent* abilityComponent = components.m_entity.GetComponent<AbilityComponent>())
		{
			abilityComponent->m_abilityToRefundId = components.m_spawningComponent->m_spawnQueue[indexToCancel].m_spawningAbilityRecordId;
		}
	}

	components.m_spawningComponent->m_spawnQueue.PopLast();
	if (indexToCancel > 0u)
	{
		return;
	}

	components.m_spawningComponent->m_spawnTimerHandle.CancelTimer();
	ProcessQueuedSpawnEntity(components);
}

bool SpawningSystems::ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessSpawningTaskCommands);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	bool spawnedAnEntityThisFrame = false;

	switch (components.m_taskComponent->m_spawningState)
	{
		case ESpawningState::ProcessQueuedSpawnEntity:
			spawnedAnEntityThisFrame = ProcessQueuedSpawnEntity(components);
			break;
		case ESpawningState::WaitingToSpawnEntity:
			if (components.m_spawningComponent->m_spawnTimerHandle.IsTimerComplete(components.m_entity))
			{
				components.m_spawningComponent->m_spawnTimerHandle.FinishTimerHandling(components.m_entity);
				components.m_taskComponent->m_spawningState = ESpawningState::SpawningEntity;

				SpawnEntityFromQueue(components);
				spawnedAnEntityThisFrame = true;
			}
			break;

		case ESpawningState::SpawningEntity:
			SpawnEntityFromQueue(components);
			spawnedAnEntityThisFrame = true;
			break;

		default:
			break;
	}

	return spawnedAnEntityThisFrame;
}

bool SpawningSystems::ProcessQueuedSpawnEntity(const SpawningSystemsArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessQueuedSpawnEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (components.m_spawningComponent->m_spawnQueue.IsEmpty())
	{
		components.m_taskComponent->m_spawningState = ESpawningState::None;
		return false;
	}

	SpawnEntityInfo spawnInfo = components.m_spawningComponent->m_spawnQueue.First();

	bool spawnedAnEntityThisFrame = false;
	if (spawnInfo.m_timeToCastSeconds > 0.0f)
	{
		components.m_taskComponent->m_spawningState = ESpawningState::WaitingToSpawnEntity;
		components.m_spawningComponent->m_spawnTimerHandle.StartTimer(components.m_entity, spawnInfo.m_timeToCastSeconds);
	}
	else
	{
		components.m_taskComponent->m_spawningState = ESpawningState::SpawningEntity;
		SpawnEntityFromQueue(components);
		spawnedAnEntityThisFrame = true;
	}

	return spawnedAnEntityThisFrame;
}

void SpawningSystems::GetSpawnLocationAndNavigationState(const SpawningSystemsArgs& components, FVector& outSpawnLocation, EMovementState& outMovementState)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const FVector spawnerLocation = components.m_transformComponent->m_location;
	outSpawnLocation = spawnerLocation;
	outMovementState = EMovementState::None;

	if (components.m_targetingComponent->HasLocationTarget())
	{
		FVector spawnForward = components.m_targetingComponent->m_targetLocation.GetValue() - spawnerLocation;
		spawnForward.Normalize();
		outSpawnLocation = spawnerLocation + (spawnForward * components.m_spawningComponent->m_spawningRadius);
		outMovementState = EMovementState::ProcessMoveToLocationCommand;
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
		FVector spawnForward = ArgusMath::GetDirectionFromYaw(components.m_transformComponent->GetCurrentYaw());
		outSpawnLocation = spawnerLocation + (spawnForward * components.m_spawningComponent->m_spawningRadius);
		outMovementState = EMovementState::None;
		return;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (!targetEntity)
	{
		return;
	}

	TransformComponent* targetTransformComponent = targetEntity.GetComponent<TransformComponent>();
	if (!targetTransformComponent)
	{
		return;
	}

	FVector spawnForward = targetTransformComponent->m_location - spawnerLocation;
	spawnForward.Normalize();
	outSpawnLocation = spawnerLocation + (spawnForward * components.m_spawningComponent->m_spawningRadius);
	outMovementState = EMovementState::ProcessMoveToEntityCommand;
}

void SpawningSystems::CommandMoveSelectedEntitiesToSpawnedEntity(const ArgusEntity& spawnedEntity, bool requireConstructionTarget)
{
	if (!spawnedEntity)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Move to %s. It is invalid."),
			ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity)
		);
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid reference to %s."),
			ARGUS_FUNCNAME, ARGUS_NAMEOF(singletonEntity)
		);
		return;
	}

	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid reference to %s."),
			ARGUS_FUNCNAME, ARGUS_NAMEOF(InputInterfaceComponent)
		);
		return;
	}

	for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		ArgusEntity selectedEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
		if (!selectedEntity)
		{
			continue;
		}

		TaskComponent* selectedEntityTaskComponent = selectedEntity.GetComponent<TaskComponent>();
		if (!selectedEntityTaskComponent)
		{
			continue;
		}

		TargetingComponent* selectedEntityTargetingComonent = selectedEntity.GetComponent<TargetingComponent>();
		if (!selectedEntityTargetingComonent)
		{
			continue;
		}

		if (requireConstructionTarget && !ConstructionSystems::CanEntityConstructOtherEntity(selectedEntity, spawnedEntity))
		{
			continue;
		}

		selectedEntityTaskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
		selectedEntityTargetingComonent->m_targetEntityId = spawnedEntity.GetId();
		selectedEntityTargetingComonent->m_targetLocation.Reset();
	}
}
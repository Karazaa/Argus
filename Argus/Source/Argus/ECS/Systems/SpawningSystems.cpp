// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpawningSystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"
#include "HAL/UnrealMemory.h"

bool SpawningSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(SpawningSystems::RunSystems);

	bool spawnedAnEntityThisFrame = false;

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		SpawningSystemsComponentArgs components;
		components.m_entity = potentialEntity;
		components.m_spawningComponent = components.m_entity.GetComponent<SpawningComponent>();
		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_targetingComponent = components.m_entity.GetComponent<TargetingComponent>();
		components.m_transformComponent = components.m_entity.GetComponent<TransformComponent>();

		if (!components.m_entity || !components.m_taskComponent || !components.m_targetingComponent || 
			!components.m_spawningComponent || !components.m_transformComponent)
		{
			continue;
		}

		if (components.m_taskComponent->m_constructionState == ConstructionState::BeingConstructed)
		{
			continue;
		}

		spawnedAnEntityThisFrame |= ProcessSpawningTaskCommands(deltaTime, components);
	}

	return spawnedAnEntityThisFrame;
}

bool SpawningSystems::SpawningSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_spawningComponent && m_targetingComponent && m_transformComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(SpawningSystemsComponentArgs));

	return false;
}

void SpawningSystems::SpawnEntity(const SpawningSystemsComponentArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord)
{
	ARGUS_TRACE(SpawningSystems::SpawnEntity);
	SpawnEntityInternal(components, spawnInfo, overrideArgusActorRecord);
	ProcessQueuedSpawnEntity(components);
}

void SpawningSystems::SpawnEntityInternal(const SpawningSystemsComponentArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord)
{
	ARGUS_TRACE(SpawningSystems::SpawnEntityInternal);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const UArgusActorRecord* argusActorRecord = overrideArgusActorRecord ? overrideArgusActorRecord : ArgusStaticData::GetRecord<UArgusActorRecord>(spawnInfo.m_argusActorRecordId);

	if (components.m_taskComponent->m_spawningState != SpawningState::SpawningEntity || !argusActorRecord || !argusActorRecord->m_entityTemplateOverride)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not spawn %s. Spawner or %s is malformed."), 
			ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(UArgusActorRecord)
		);
		return;
	}

	components.m_taskComponent->m_spawningState = SpawningState::None;

	ArgusEntity spawnedEntity = argusActorRecord->m_entityTemplateOverride->MakeEntity();
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

	spawnedEntityTaskComponent->m_baseState = BaseState::SpawnedWaitingForActorTake;
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

	FVector spawnLocation = components.m_transformComponent->m_transform.GetLocation();
	MovementState initialSpawnMovementState = MovementState::None;

	if (spawnInfo.m_spawnLocationOverride.IsSet())
	{
		spawnLocation = spawnInfo.m_spawnLocationOverride.GetValue();
	}
	else
	{
		GetSpawnLocationAndNavigationState(components, spawnLocation, initialSpawnMovementState);
	}

	spawnedEntityTransformComponent->m_transform.SetLocation(spawnLocation);

	if (spawnInfo.m_needsConstruction)
	{
		spawnedEntityTaskComponent->m_constructionState = ConstructionState::BeingConstructed;
		if (ConstructionComponent* constructionComponent = spawnedEntity.GetOrAddComponent<ConstructionComponent>())
		{
			constructionComponent->m_constructionAbilityRecordId = argusActorRecord->m_id;
			constructionComponent->m_automaticConstructionTimerHandle.StartTimer(spawnedEntity, constructionComponent->m_requiredWorkSeconds);
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

void SpawningSystems::SpawnEntityFromQueue(const SpawningSystemsComponentArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_spawningState != SpawningState::SpawningEntity)
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] The %s of the spawning %s is not %s"), 
			ARGUS_FUNCNAME, ARGUS_NAMEOF(m_spawningState), ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(SpawningState::SpawningEntity)
		);
		return;
	}

	SpawnEntityInfo spawnInfo;
	if (!components.m_spawningComponent->m_spawnQueue.Dequeue(spawnInfo))
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not dequeue a %s from the spawner's %s."),
			ARGUS_FUNCNAME, ARGUS_NAMEOF(SpawnEntityInfo), ARGUS_NAMEOF(SpawningComponent)
		);
		return;
	}

	components.m_spawningComponent->m_currentQueueSize--;
	SpawnEntity(components, spawnInfo);
}

bool SpawningSystems::ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsComponentArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessSpawningTaskCommands);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	bool spawnedAnEntityThisFrame = false;

	switch (components.m_taskComponent->m_spawningState)
	{
		case SpawningState::ProcessQueuedSpawnEntity:
			spawnedAnEntityThisFrame = ProcessQueuedSpawnEntity(components);
			break;
		case SpawningState::WaitingToSpawnEntity:
			if (components.m_spawningComponent->m_spawnTimerHandle.IsTimerComplete(components.m_entity))
			{
				components.m_spawningComponent->m_spawnTimerHandle.FinishTimerHandling(components.m_entity);
				components.m_taskComponent->m_spawningState = SpawningState::SpawningEntity;

				SpawnEntityFromQueue(components);
				spawnedAnEntityThisFrame = true;
			}
			break;

		case SpawningState::SpawningEntity:
			SpawnEntityFromQueue(components);
			spawnedAnEntityThisFrame = true;
			break;

		default:
			break;
	}

	return spawnedAnEntityThisFrame;
}

bool SpawningSystems::ProcessQueuedSpawnEntity(const SpawningSystemsComponentArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessQueuedSpawnEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (components.m_spawningComponent->m_spawnQueue.IsEmpty())
	{
		components.m_taskComponent->m_spawningState = SpawningState::None;
		return false;
	}

	SpawnEntityInfo spawnInfo;
	if (!components.m_spawningComponent->m_spawnQueue.Peek(spawnInfo))
	{
		ARGUS_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] Could not peek a %s from the spawner's %s."),
			ARGUS_FUNCNAME, ARGUS_NAMEOF(SpawnEntityInfo), ARGUS_NAMEOF(SpawningComponent)
		);
		return false;
	}

	bool spawnedAnEntityThisFrame = false;
	if (spawnInfo.m_timeToCastSeconds > 0.0f)
	{
		components.m_taskComponent->m_spawningState = SpawningState::WaitingToSpawnEntity;
		components.m_spawningComponent->m_spawnTimerHandle.StartTimer(components.m_entity, spawnInfo.m_timeToCastSeconds);
	}
	else
	{
		components.m_taskComponent->m_spawningState = SpawningState::SpawningEntity;
		SpawnEntityFromQueue(components);
		spawnedAnEntityThisFrame = true;
	}

	return spawnedAnEntityThisFrame;
}

void SpawningSystems::GetSpawnLocationAndNavigationState(const SpawningSystemsComponentArgs& components, FVector& outSpawnLocation, MovementState& outMovementState)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const FVector spawnerLocation = components.m_transformComponent->m_transform.GetLocation();
	outSpawnLocation = spawnerLocation;
	outMovementState = MovementState::None;

	if (components.m_targetingComponent->HasLocationTarget())
	{
		FVector spawnForward = components.m_targetingComponent->m_targetLocation.GetValue() - spawnerLocation;
		spawnForward.Normalize();
		outSpawnLocation = spawnerLocation + (spawnForward * components.m_spawningComponent->m_spawningRadius);
		outMovementState = MovementState::ProcessMoveToLocationCommand;
		return;
	}

	if (!components.m_targetingComponent->HasEntityTarget())
	{
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

	FVector spawnForward = targetTransformComponent->m_transform.GetLocation() - spawnerLocation;
	spawnForward.Normalize();
	outSpawnLocation = spawnerLocation + (spawnForward * components.m_spawningComponent->m_spawningRadius);
	outMovementState = MovementState::ProcessMoveToEntityCommand;
}
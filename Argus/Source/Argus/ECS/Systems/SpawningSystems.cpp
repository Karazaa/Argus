// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpawningSystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"

void SpawningSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(SpawningSystems::RunSystems)

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

		ProcessSpawningTaskCommands(deltaTime, components);
	}
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

void SpawningSystems::SpawnEntity(const SpawningSystemsComponentArgs& components, const UArgusActorRecord* argusActorRecord)
{
	ARGUS_TRACE(SpawningSystems::SpawnEntity)

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_spawningState != ESpawningState::ProcessSpawnEntityCommand || !argusActorRecord || !argusActorRecord->m_entityTemplateOverride)
	{
		return;
	}

	ArgusEntity spawnedEntity = argusActorRecord->m_entityTemplateOverride->MakeEntity();
	TaskComponent* spawnedEntityTaskComponent = spawnedEntity.GetOrAddComponent<TaskComponent>();
	if (!spawnedEntityTaskComponent)
	{
		return;
	}

	spawnedEntityTaskComponent->m_baseState = EBaseState::SpawnedWaitingForActorTake;
	spawnedEntityTaskComponent->m_spawnedFromArgusActorRecordId = argusActorRecord->m_id;

	components.m_taskComponent->m_spawningState = ESpawningState::None;

	TransformComponent* spawnedEntityTransformComponent = spawnedEntity.GetComponent<TransformComponent>();
	if (!spawnedEntityTransformComponent)
	{
		return;
	}

	FVector spawnLocation = components.m_transformComponent->m_transform.GetLocation();
	EMovementState initialSpawnMovementState = EMovementState::None;
	GetSpawnLocationAndNavigationState(components, spawnLocation, initialSpawnMovementState);

	spawnedEntityTransformComponent->m_transform.SetLocation(spawnLocation);

	NavigationComponent* spawnedEntityNavigationComponent = spawnedEntity.GetComponent<NavigationComponent>();
	TargetingComponent* spawnedEntityTargetingComponent = spawnedEntity.GetComponent<TargetingComponent>();
	if (!spawnedEntityNavigationComponent || !spawnedEntityTargetingComponent)
	{
		return;
	}

	spawnedEntityTaskComponent->m_movementState = initialSpawnMovementState;
	*spawnedEntityTargetingComponent = *components.m_targetingComponent;
}

void SpawningSystems::ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsComponentArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessSpawningTaskCommands)

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_spawningState)
	{
		case ESpawningState::ProcessSpawnEntityCommand:
			SpawnEntity(components, ArgusStaticData::GetRecord<UArgusActorRecord>(components.m_spawningComponent->m_argusActorRecordId));
			break;

		default:
			break;
	}
}

void SpawningSystems::GetSpawnLocationAndNavigationState(const SpawningSystemsComponentArgs& components, FVector& outSpawnLocation, EMovementState& outMovementState)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const FVector spawnerLocation = components.m_transformComponent->m_transform.GetLocation();
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
	outSpawnLocation = spawnerLocation + (spawnForward * components.m_transformComponent->m_radius);
	outMovementState = EMovementState::ProcessMoveToEntityCommand;
}
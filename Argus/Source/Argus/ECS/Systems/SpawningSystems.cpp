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
		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_spawningComponent = components.m_entity.GetComponent<SpawningComponent>();

		if (!components.AreComponentsValidCheck(false))
		{
			continue;
		}

		ProcessSpawningTaskCommands(deltaTime, components);
	}
}

bool SpawningSystems::SpawningSystemsComponentArgs::AreComponentsValidCheck(bool shouldErrorIfInvalid, const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_spawningComponent)
	{
		return true;
	}

	if (shouldErrorIfInvalid && functionName)
	{
		ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(SpawningSystemsComponentArgs));
	}

	return false;
}

void SpawningSystems::SpawnEntity(const SpawningSystemsComponentArgs& components, const UArgusActorRecord* argusActorRecord)
{
	ARGUS_TRACE(SpawningSystems::SpawnEntity)

	if (!components.AreComponentsValidCheck(true, ARGUS_FUNCNAME))
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
}

void SpawningSystems::ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsComponentArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessSpawningTaskCommands)

	if (!components.AreComponentsValidCheck(true, ARGUS_FUNCNAME))
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
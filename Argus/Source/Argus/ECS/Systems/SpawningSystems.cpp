// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpawningSystems.h"
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

		if (!components.AreComponentsValidCheck())
		{
			continue;
		}

		ProcessSpawningTaskCommands(deltaTime, components);
	}
}

bool SpawningSystems::SpawningSystemsComponentArgs::AreComponentsValidCheck() const
{
	return m_entity && m_taskComponent && m_spawningComponent;
}

void SpawningSystems::SpawnEntity(TaskComponent* spawningTaskComponent, const UArgusActorRecord* argusActorRecord)
{
	ARGUS_TRACE(SpawningSystems::SpawnEntity)

	if (!spawningTaskComponent || spawningTaskComponent->m_spawningState != ESpawningState::ProcessSpawnEntityCommand || !argusActorRecord || !argusActorRecord->m_entityTemplateOverride)
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

	spawningTaskComponent->m_spawningState = ESpawningState::None;
}

void SpawningSystems::ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsComponentArgs& components)
{
	ARGUS_TRACE(SpawningSystems::ProcessSpawningTaskCommands)

	if (!components.AreComponentsValidCheck())
	{
		return;
	}

	switch (components.m_taskComponent->m_spawningState)
	{
		case ESpawningState::None:
			break;
		case ESpawningState::ProcessSpawnEntityCommand:
			SpawnEntity(components.m_taskComponent, ArgusStaticData::GetRecord<UArgusActorRecord>(components.m_spawningComponent->m_argusActorRecordId));
			break;
		default:
			break;
	}
}
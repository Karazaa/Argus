// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

const ArgusEntity ArgusEntity::s_emptyEntity = ArgusEntity();
std::bitset<ArgusECSConstants::k_maxEntities> ArgusEntity::s_takenEntityIds = std::bitset<ArgusECSConstants::k_maxEntities>();

uint16 ArgusEntity::s_lowestTakenEntityId = ArgusECSConstants::k_maxEntities;
uint16 ArgusEntity::s_highestTakenEntityId = 0u;

void ArgusEntity::FlushAllEntities()
{
	ArgusComponentRegistry::FlushAllComponents();
	s_takenEntityIds.reset();
	s_lowestTakenEntityId = ArgusECSConstants::k_maxEntities;
	s_highestTakenEntityId = 0u;
}

bool ArgusEntity::DoesEntityExist(uint16 id)
{
	if (id >= ArgusECSConstants::k_maxEntities)
	{
		return false;
	}

	return s_takenEntityIds[id];
}

ArgusEntity ArgusEntity::CreateEntity(uint16 lowestId)
{
	const uint16 id = GetNextLowestUntakenId(lowestId);

	if (id == ArgusECSConstants::k_maxEntities)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to create an %s with an invalid ID value."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return s_emptyEntity;
	}

	if (s_takenEntityIds[id])
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to create an %s with an ID value of an already existing %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity));
		return s_emptyEntity;
	}

	s_takenEntityIds.set(id);
	if (id < s_lowestTakenEntityId)
	{
		s_lowestTakenEntityId = id;
	}
	if (id > s_highestTakenEntityId)
	{
		s_highestTakenEntityId = id;
	}

	return ArgusEntity(id);
}

void ArgusEntity::DestroyEntity(ArgusEntity& entityToDestroy)
{
	if (!entityToDestroy)
	{
		return;
	}

	uint16 entityToDestoryId = entityToDestroy.GetId();
	s_takenEntityIds.set(entityToDestoryId, false);
	ArgusComponentRegistry::RemoveComponentsForEntity(entityToDestoryId);
	
	if (entityToDestoryId == s_lowestTakenEntityId)
	{
		uint16 newLowestTakenId = ArgusECSConstants::k_maxEntities;
		for (uint16 i = s_lowestTakenEntityId + 1u; i <= s_highestTakenEntityId; ++i)
		{
			if (s_takenEntityIds[entityToDestoryId])
			{
				newLowestTakenId = i;
				break;
			}
		}

		s_lowestTakenEntityId = newLowestTakenId;
	}

	if (entityToDestoryId == s_highestTakenEntityId)
	{
		uint16 newHighestTakenId = 0u;
		for (uint16 i = s_highestTakenEntityId - 1u; i >= s_lowestTakenEntityId; --i)
		{
			if (s_takenEntityIds[entityToDestoryId])
			{
				newHighestTakenId = i;
				break;
			}
		}

		s_highestTakenEntityId = newHighestTakenId;
	}

	entityToDestroy = s_emptyEntity;
}

void ArgusEntity::DestroyEntity(uint16 entityIdToDestroy)
{
	ArgusEntity entityToDestroy = RetrieveEntity(entityIdToDestroy);
	if (entityToDestroy)
	{
		DestroyEntity(entityToDestroy);
	}
}

ArgusEntity ArgusEntity::RetrieveEntity(uint16 id)
{
	if (id < ArgusECSConstants::k_maxEntities && s_takenEntityIds[id])
	{
		return ArgusEntity(id);
	}

	return ArgusEntity::s_emptyEntity;
}

uint16 ArgusEntity::GetNextLowestUntakenId(uint16 lowestId)
{
	if (lowestId >= ArgusECSConstants::k_maxEntities)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to retrieve an ID that is beyond %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusECSConstants::k_maxEntities));
		return ArgusECSConstants::k_maxEntities;
	}

	while (lowestId < (ArgusECSConstants::k_maxEntities - 1) && s_takenEntityIds[lowestId])
	{
		lowestId++;
	}

	if (s_takenEntityIds[lowestId])
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Exceeded the maximum number of allowed %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return ArgusECSConstants::k_maxEntities;
	}

	return lowestId;
}

ArgusEntity::ArgusEntity(const ArgusEntity& other)
{
	m_id = other.GetId();
}

ArgusEntity& ArgusEntity::operator=(const ArgusEntity& other)
{
	m_id = other.GetId();
	return *this;
}

bool ArgusEntity::operator==(const ArgusEntity& other) const
{
	return m_id == other.GetId();
}

ArgusEntity::operator bool() const
{
	return DoesEntityExist(m_id);
}

ArgusEntity::ArgusEntity() : m_id(ArgusECSConstants::k_maxEntities)
{
}

ArgusEntity::ArgusEntity(uint16 id) : m_id(id)
{
}

uint16 ArgusEntity::GetId() const
{
	return m_id;
}

const FString ArgusEntity::GetDebugString() const
{
	FString debugString = FString::Printf(TEXT("(%s: %d)"), ARGUS_NAMEOF(m_id), m_id);
	ArgusComponentRegistry::AppendComponentDebugStrings(m_id, debugString);
	return debugString;
}

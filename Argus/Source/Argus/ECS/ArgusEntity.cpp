// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

const ArgusEntity ArgusEntity::s_emptyEntity = ArgusEntity();
std::bitset<ArgusECSConstants::k_maxEntities> ArgusEntity::s_takenEntityIds = std::bitset<ArgusECSConstants::k_maxEntities>();

uint16 ArgusEntity::s_lowestTakenEntityId = ArgusECSConstants::k_maxEntities;
uint16 ArgusEntity::s_highestTakenEntityId = 0u;

ArgusEntity ArgusEntity::CreateEntity(uint16 lowestId)
{
	const uint16 id = GetLowestValidId(lowestId);

	if (s_takenEntityIds[id])
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to create an %s with an ID value of an already existing %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity));
		return s_emptyEntity;
	}

	if (id == ArgusECSConstants::k_maxEntities)
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] Attempting to create an %s with an invalid ID value."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
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

bool ArgusEntity::DoesEntityExist(uint16 id)
{
	if (id >= ArgusECSConstants::k_maxEntities)
	{
		return false;
	}

	return s_takenEntityIds[id];
}

ArgusEntity ArgusEntity::RetrieveEntity(uint16 id)
{
	if (id < ArgusECSConstants::k_maxEntities && s_takenEntityIds[id])
	{
		return ArgusEntity(id);
	}

	return ArgusEntity::s_emptyEntity;
}

void ArgusEntity::FlushAllEntities()
{
	ArgusComponentRegistry::FlushAllComponents();
	s_takenEntityIds.reset();
	s_lowestTakenEntityId = ArgusECSConstants::k_maxEntities;
	s_highestTakenEntityId = 0u;
}

uint16 ArgusEntity::GetLowestValidId(uint16 lowestId)
{
	if (lowestId >= ArgusECSConstants::k_maxEntities)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	while (lowestId < (ArgusECSConstants::k_maxEntities - 1) && s_takenEntityIds[lowestId])
	{
		lowestId++;
	}

	if (s_takenEntityIds[lowestId])
	{
		UE_LOG(ArgusECSLog, Error, TEXT("[%s] Exceeded the maximum number of allowed %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
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

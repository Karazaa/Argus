// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusUtil.h"

const ArgusEntity ArgusEntity::s_emptyEntity = ArgusEntity();
std::bitset<ArgusECSConstants::k_maxEntities> ArgusEntity::s_takenEntityIds = std::bitset<ArgusECSConstants::k_maxEntities>();

ArgusEntity ArgusEntity::CreateEntity(uint16 lowestId)
{
	return ArgusEntity(GetLowestValidId(lowestId));
}

bool ArgusEntity::DoesEntityExist(uint16 id)
{
	if (id >= ArgusECSConstants::k_maxEntities)
	{
		return false;
	}

	return s_takenEntityIds[id];
}

std::optional<ArgusEntity> ArgusEntity::RetrieveEntity(uint16 id)
{
	if (id < ArgusECSConstants::k_maxEntities && s_takenEntityIds[id])
	{
		return ArgusEntity(id);
	}

	return std::nullopt;
}

void ArgusEntity::FlushAllEntities()
{
	ArgusComponentRegistry::FlushAllComponents();
	s_takenEntityIds.reset();
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
		UE_LOG(ArgusGameLog, Error, TEXT("[ECS] Exceeded the maximum number of allowed entities."));
	}

	s_takenEntityIds.set(lowestId);
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

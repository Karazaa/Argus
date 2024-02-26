// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusUtil.h"

std::bitset<ArgusECSConstants::k_maxEntities> ArgusEntity::s_takenEntityIds = std::bitset<ArgusECSConstants::k_maxEntities>();

ArgusEntity ArgusEntity::CreateEntity(uint16 lowestId)
{
	return ArgusEntity(GetLowestValidId(lowestId));
}

bool ArgusEntity::DoesEntityExist(uint16 id)
{
	return s_takenEntityIds[id];
}

std::optional<ArgusEntity> ArgusEntity::RetrieveEntity(uint16 id)
{
	if (s_takenEntityIds[id])
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
	while (s_takenEntityIds[lowestId] && lowestId < ArgusECSConstants::k_maxEntities)
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

ArgusEntity& ArgusEntity::operator=(ArgusEntity other)
{
	m_id = other.GetId();
	return *this;
}

ArgusEntity::ArgusEntity() : m_id(0)
{
}

ArgusEntity::ArgusEntity(uint16 id) : m_id(id)
{
}

uint16 ArgusEntity::GetId() const
{
	return m_id;
}

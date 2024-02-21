// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusUtil.h"

std::bitset<ArgusEntity::k_maxEntities> ArgusEntity::s_takenEntityIds = std::bitset<ArgusEntity::k_maxEntities>();

void ArgusEntity::FlushAllEntities()
{
	s_takenEntityIds.reset();
}

uint16 ArgusEntity::GetLowestValidId(uint16 lowestPotentialId)
{
	while (s_takenEntityIds[lowestPotentialId] && lowestPotentialId < k_maxEntities)
	{
		lowestPotentialId++;
	}

	if (s_takenEntityIds[lowestPotentialId])
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[ECS] Exceeded the maximum number of allowed entities!"));
	}

	s_takenEntityIds.set(lowestPotentialId);
	return lowestPotentialId;
}

ArgusEntity::ArgusEntity()
{
	m_id = ArgusEntity::GetLowestValidId(0);
}

ArgusEntity::ArgusEntity(uint16 lowestPotentialId)
{
	m_id = ArgusEntity::GetLowestValidId(lowestPotentialId);
}

uint16 ArgusEntity::GetId()
{
	return m_id;
}

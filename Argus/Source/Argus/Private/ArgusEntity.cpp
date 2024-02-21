// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"

ArgusEntity::ArgusEntity(uint32_t id) : m_id(id)
{
}

ArgusEntity::~ArgusEntity()
{
}

uint32_t ArgusEntity::GetId()
{
	return m_id;
}

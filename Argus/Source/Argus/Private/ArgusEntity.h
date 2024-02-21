// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include <bitset>

class ArgusEntity
{
public:
	static void FlushAllEntities();
private:
	static constexpr uint16 k_maxEntities = MAX_uint16;
	static std::bitset<k_maxEntities> s_takenEntityIds;

	static uint16 GetLowestValidId(uint16 lowestId);
	
public:
	ArgusEntity();
	ArgusEntity(uint16 lowestId);
	uint16 GetId();
private:
	uint16 m_id;
};

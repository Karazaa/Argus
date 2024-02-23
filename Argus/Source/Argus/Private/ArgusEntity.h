// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include <bitset>
#include <optional>

class ArgusEntity
{
public:
	static ArgusEntity					CreateEntity(uint16 lowestId = 0u);
	static bool							DoesEntityExist(uint16 id);
	static std::optional<ArgusEntity>	RetrieveEntity(uint16 id);
	static void							FlushAllEntities();

private:
	static constexpr uint16				k_maxEntities = MAX_uint16;
	static std::bitset<k_maxEntities>	s_takenEntityIds;

	static uint16						GetLowestValidId(uint16 lowestId);
	
public:
	ArgusEntity(const ArgusEntity& other);
	ArgusEntity& operator=(ArgusEntity other);

	uint16	GetId() const;

	template<class Component>
	std::optional<Component> GetComponent();

private:
	ArgusEntity();
	ArgusEntity(uint16 id);

	uint16 m_id;
};

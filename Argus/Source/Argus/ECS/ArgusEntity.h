// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusComponentRegistry.h"
#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include <bitset>

class ArgusEntity
{
public:
	static ArgusEntity					CreateEntity(uint16 lowestId = 0u);
	static bool							DoesEntityExist(uint16 id);
	static ArgusEntity					RetrieveEntity(uint16 id);
	static void							FlushAllEntities();
	static uint16						GetLowestTakenEntityId() { return s_lowestTakenEntityId; }
	static uint16						GetHighestTakenEntityId() { return s_highestTakenEntityId; }

	static const ArgusEntity			s_emptyEntity;

private:
	static std::bitset<ArgusECSConstants::k_maxEntities>	s_takenEntityIds;
	static uint16 s_lowestTakenEntityId;
	static uint16 s_highestTakenEntityId;

	static uint16						GetLowestValidId(uint16 lowestId);
	
public:
	ArgusEntity(const ArgusEntity& other);
	ArgusEntity& operator=(const ArgusEntity& other);
	bool operator==(const ArgusEntity& other) const;
	operator bool() const;

	uint16	GetId() const;

	template<class ArgusComponent>
	ArgusComponent* GetComponent() const
	{
		return ArgusComponentRegistry::GetComponent<ArgusComponent>(m_id);
	}

	template<class ArgusComponent>
	ArgusComponent* AddComponent() const
	{
		return ArgusComponentRegistry::AddComponent<ArgusComponent>(m_id);
	}

	template<class ArgusComponent>
	ArgusComponent* GetOrAddComponent() const
	{
		return ArgusComponentRegistry::GetOrAddComponent<ArgusComponent>(m_id);
	}

private:
	ArgusEntity();
	ArgusEntity(uint16 id);

	uint16 m_id;
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusComponentRegistry.h"
#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include <bitset>

class ArgusEntity
{
public:
	static void				FlushAllEntities();
	static bool				DoesEntityExist(uint16 id);
	static bool				IsReservedEntityId(uint16 id);
	static ArgusEntity		CreateEntity(uint16 lowestId = 0u);
	static void				DestroyEntity(ArgusEntity& entityToDestroy);
	static void				DestroyEntity(uint16 entityIdToDestroy);
	static ArgusEntity		RetrieveEntity(uint16 id);
	static uint16			GetNextLowestUntakenId(uint16 lowestId);
	static uint16			GetLowestTakenEntityId() { return s_lowestTakenEntityId; }
	static uint16			GetHighestTakenEntityId() { return s_highestTakenEntityId; }
	static ArgusEntity		GetSingletonEntity() { return RetrieveEntity(ArgusECSConstants::k_singletonEntityId); }
	static uint16			GetTeamEntityId(ETeam team);
	static ArgusEntity		GetTeamEntity(ETeam team);

	static const ArgusEntity k_emptyEntity;

private:
	static std::bitset<ArgusECSConstants::k_maxEntities>	s_takenEntityIds;
	static uint16 s_lowestTakenEntityId;
	static uint16 s_highestTakenEntityId;
	
public:
	ArgusEntity(const ArgusEntity& other);
	ArgusEntity& operator=(const ArgusEntity& other);
	bool operator==(const ArgusEntity& other) const;
	operator bool() const;

	uint16			GetId() const;
	const FString	GetDebugString() const;
	bool			IsKillable() const;
	bool			IsAlive() const;
	bool			IsMoveable() const;
	bool			IsSelected() const;

	template<class ArgusComponent>
	inline ArgusComponent* GetComponent() const
	{
		return ArgusComponentRegistry::GetComponent<ArgusComponent>(m_id);
	}

	template<class ArgusComponent>
	inline ArgusComponent* AddComponent() const
	{
		return ArgusComponentRegistry::AddComponent<ArgusComponent>(m_id);
	}

	template<class ArgusComponent>
	inline ArgusComponent* GetOrAddComponent() const
	{
		return ArgusComponentRegistry::GetOrAddComponent<ArgusComponent>(m_id);
	}

private:
	ArgusEntity();
	ArgusEntity(uint16 id);

	uint16 m_id;
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusComponentRegistry.h"
#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include <bitset>

class FArchive;
class UArgusActorRecord;

class ARGUS_API ArgusEntity
{
private:
	static TBitArray<ArgusContainerAllocator<ArgusECSConstants::k_numBitBuckets> > s_takenEntityIds;

public:
	static int32			FindFromEntityBitArray(bool status, int32 index) { return s_takenEntityIds.FindFrom(status, index); }
	static void				FlushAllEntities();
	static void				Serialize(FArchive& archive);
	static bool				DoesEntityExist(uint16 id);
	static bool				IsReservedEntityId(uint16 id);
	static uint16			GetHighestNonReservedEntityId();
	static ArgusEntity		CreateEntity(uint16 lowestId = 0u);
	static void				DestroyEntity(ArgusEntity& entityToDestroy);
	static void				DestroyEntity(uint16 entityIdToDestroy);
	static ArgusEntity		RetrieveEntity(uint16 id);
	static uint16			GetNextLowestUntakenId(uint16 lowestId);
	static uint16			GetLowestTakenEntityId() { return s_lowestTakenEntityId; }
	static uint16			GetHighestTakenEntityId() { return s_highestTakenEntityId; }
	static ArgusEntity		GetSingletonEntity();
	static uint16			GetTeamOffset(ETeam team);
	static uint16			GetTeamEntityId(ETeam team);
	static ArgusEntity		GetTeamEntity(ETeam team);
	static void				RegisterTeam(ETeam team);
	static bool				IsTeamRegistered(ETeam team);

	static const ArgusEntity k_emptyEntity;

private:
	static uint16 s_lowestTakenEntityId;
	static uint16 s_highestTakenEntityId;
	static BITMASK_ETeam s_teamsForIteration;
	
public:
	FORCEINLINE ArgusEntity(const ArgusEntity& other) { m_id = other.GetId(); };
	ArgusEntity& operator=(const ArgusEntity& other);
	bool operator==(const ArgusEntity& other) const;
	operator bool() const;

	FORCEINLINE uint16	GetId() const { return m_id; };
	bool				IsKillable() const;
	bool				IsAlive() const;
	bool				IsMoveable() const;
	bool				IsSelected() const;
	bool				IsIdle() const;
	bool				IsInRangeOfOtherEntity(ArgusEntity otherEntity, float range) const;
	bool				IsInRangeOfTargetEntity() const;
	bool				IsPassenger() const;
	bool				IsCarryingPassengers() const;
	bool				IsOnTeam(ETeam team) const;
	bool				IsOnSameTeamAsOtherEntity(ArgusEntity otherEntity) const;
	bool				IsOtherEntityAnEnemy(ArgusEntity otherEntity) const;
	bool				IsOtherEntityAnAlly(ArgusEntity otherEntity) const;
	bool				IsOnPlayerTeam() const;
	bool				IsFlying() const;
	bool				IsUnderConstruction() const;
	bool				CanFly() const;
	bool				DoesEntitySatisfyEntityCategory(EntityCategory entityCategory) const;
	FVector				GetCurrentTargetLocation() const;
	float				GetDistanceSquaredToOtherEntity(ArgusEntity otherEntity) const;
	float				GetDistanceToOtherEntity(ArgusEntity otherEntity) const;

	const UArgusActorRecord* GetAssociatedActorRecord() const;

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

	void Destroy();

private:
	ArgusEntity();
	ArgusEntity(uint16 id);

	uint16 m_id;
};

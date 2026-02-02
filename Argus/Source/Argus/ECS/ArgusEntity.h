// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusComponentRegistry.h"
#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include <bitset>

class UArgusActorRecord;

class ArgusEntity
{
private:
	static TBitArray<ArgusContainerAllocator<ArgusECSConstants::k_numBitBuckets> > s_takenEntityIds;

public:
	static void				FlushAllEntities();
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
	static ArgusEntity		GetSingletonEntity() { return RetrieveEntity(ArgusECSConstants::k_singletonEntityId); }
	static uint16			GetTeamOffset(ETeam team);
	static uint16			GetTeamEntityId(ETeam team);
	static ArgusEntity		GetTeamEntity(ETeam team);

	template <typename Function>
	static void IterateEntities(Function&& perEntityFunction)
	{
		int32 currentIndex = GetLowestTakenEntityId();
		while (currentIndex <= GetHighestTakenEntityId() && currentIndex >= 0)
		{
			if (ArgusEntity entity = RetrieveEntity(currentIndex))
			{
				perEntityFunction(entity);
			}

			currentIndex = s_takenEntityIds.FindFrom(true, currentIndex + 1);
		}
	}

	template <typename Function>
	static void IterateTeamEntities(Function&& perEntityFunction)
	{
		for (uint8 i = 1u; i <= (sizeof(ETeam) * 8u); ++i)
		{
			if (ArgusEntity entity = RetrieveEntity(ArgusECSConstants::k_singletonEntityId - i))
			{
				perEntityFunction(entity);
			}
		}
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemsArgs(Function&& perSystemsArgsFunction)
	{
		int32 currentIndex = GetLowestTakenEntityId();
		SystemsArgs systemsArgs = SystemsArgs();

		while (currentIndex <= GetHighestTakenEntityId() && currentIndex >= 0)
		{
			if (systemsArgs.PopulateArguments(RetrieveEntity(currentIndex)))
			{
				perSystemsArgsFunction(systemsArgs);
			}

			currentIndex = s_takenEntityIds.FindFrom(true, currentIndex + 1);
		}
	}

	static const ArgusEntity k_emptyEntity;

private:
	
	static uint16 s_lowestTakenEntityId;
	static uint16 s_highestTakenEntityId;
	
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
	bool				IsPassenger() const;
	bool				IsCarryingPassengers() const;
	bool				IsOnTeam(ETeam team) const;
	bool				IsOnSameTeamAsOtherEntity(ArgusEntity otherEntity) const;
	bool				IsOnPlayerTeam() const;
	bool				IsFlying() const;
	bool				CanFly() const;
	FVector				GetCurrentTargetLocation() const;
	float				GetDistanceSquaredToOtherEntity(ArgusEntity otherEntity) const;
	float				GetDistanceToOtherEntity(ArgusEntity otherEntity) const;

	const UArgusActorRecord* GetAssociatedActorRecord() const;

#if !UE_BUILD_SHIPPING
	const FString	GetDebugString() const;
#endif //!UE_BUILD_SHIPPING

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

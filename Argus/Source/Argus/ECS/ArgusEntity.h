// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusComponentRegistry.h"
#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include "Tasks/Task.h"
#include <bitset>

class FArchive;
class UArgusActorRecord;

class ArgusEntity
{
private:
	static TBitArray<ArgusContainerAllocator<ArgusECSConstants::k_numBitBuckets> > s_takenEntityIds;

public:
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

	template <typename Function>
	static void IterateEntityRange(int32 fromInclusive, int32 toIncusive, Function&& perEntityFunction)
	{
		int32 currentIndex = fromInclusive;
		while (currentIndex <= toIncusive && currentIndex >= 0)
		{
			if (ArgusEntity entity = RetrieveEntity(currentIndex))
			{
				perEntityFunction(entity);
			}

			currentIndex = s_takenEntityIds.FindFrom(true, currentIndex + 1);
		}
	}

	template <typename Function>
	static void IterateEntities(Function&& perEntityFunction)
	{
		IterateEntityRange(GetLowestTakenEntityId(), GetHighestTakenEntityId(), perEntityFunction);
	}

	template <uint8 ChunkCount, typename Function>
	static void IterateEntitiesParallel(Function&& perEntityFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<ChunkCount>> chunkTasks;

		uint16 rollingBound = GetLowestTakenEntityId();
		const uint16 upperBound = GetHighestTakenEntityId();
		const uint16 difference = upperBound - rollingBound;
		const uint16 increment = difference / ChunkCount;

		for (uint8 i = 1u; i < ChunkCount; ++i)
		{
			chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusEntity::IterateEntitiesParallel), [rollingBound, increment, &perEntityFunction]()
			{
				IterateEntityRange(rollingBound, rollingBound + (increment - 1u), perEntityFunction);
			}));
			rollingBound += increment;
		}

		chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusEntity::IterateEntitiesParallel), [rollingBound, upperBound, &perEntityFunction]()
		{
			IterateEntityRange(rollingBound, upperBound, perEntityFunction);
		}));

		UE::Tasks::Wait(chunkTasks);
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

	template <typename Function>
	static void IterateTeamEntitiesParallel(Function&& perEntityFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<(sizeof(ETeam) * 8u)>> teamTasks;
		for (uint8 i = 1u; i <= (sizeof(ETeam) * 8u); ++i)
		{
			if (ArgusEntity entity = RetrieveEntity(ArgusECSConstants::k_singletonEntityId - i))
			{
				teamTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusEntity::IterateTeamEntitiesParallel), [entity, &perEntityFunction]()
				{
					perEntityFunction(entity);
				}));
			}
		}

		UE::Tasks::Wait(teamTasks);
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemArgRange(int32 fromInclusive, int32 toIncusive, Function&& perSystemsArgsFunction)
	{
		int32 currentIndex = fromInclusive;
		SystemsArgs systemsArgs = SystemsArgs();

		while (currentIndex <= toIncusive && currentIndex >= 0)
		{
			if (systemsArgs.PopulateArguments(RetrieveEntity(currentIndex)))
			{
				perSystemsArgsFunction(systemsArgs);
			}

			currentIndex = s_takenEntityIds.FindFrom(true, currentIndex + 1);
		}
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemsArgRangeForTeam(ETeam team, int32 fromInclusive, int32 toIncusive, Function&& perSystemsArgsFunction)
	{
		int32 currentIndex = fromInclusive;
		SystemsArgs systemsArgs = SystemsArgs();

		while (currentIndex <= toIncusive && currentIndex >= 0)
		{
			if (systemsArgs.PopulateArguments(RetrieveEntity(currentIndex)))
			{
				if (systemsArgs.m_entity.IsOnTeam(team))
				{
					perSystemsArgsFunction(systemsArgs);
				}
			}

			currentIndex = s_takenEntityIds.FindFrom(true, currentIndex + 1);
		}
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemsArgs(Function&& perSystemsArgsFunction)
	{
		IterateSystemArgRange<SystemsArgs>(GetLowestTakenEntityId(), GetHighestTakenEntityId(), perSystemsArgsFunction);
	}

	template <typename SystemsArgs, uint8 ChunkCount, typename Function>
	static void IterateSystemsArgsParallel(Function&& perSystemsArgsFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<ChunkCount>> chunkTasks;

		uint16 rollingBound = GetLowestTakenEntityId();
		const uint16 upperBound = GetHighestTakenEntityId();
		const uint16 difference = upperBound - rollingBound;
		const uint16 increment = difference / ChunkCount;

		for (uint8 i = 1u; i < ChunkCount; ++i)
		{
			chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusEntity::IterateSystemsArgsParallel), [rollingBound, increment, &perSystemsArgsFunction]()
			{
				IterateSystemArgRange<SystemsArgs>(rollingBound, rollingBound + (increment - 1u), perSystemsArgsFunction);
			}));
			rollingBound += increment;
		}

		chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusEntity::IterateSystemsArgsParallel), [rollingBound, upperBound, &perSystemsArgsFunction]()
		{
			IterateSystemArgRange<SystemsArgs>(rollingBound, upperBound, perSystemsArgsFunction);
		}));

		UE::Tasks::Wait(chunkTasks);
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemsArgsByTeamParallel(Function&& perSystemsArgsFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<(sizeof(ETeam) * 8u)>> teamTasks;
		for (uint8 i = 0u; i <= (sizeof(ETeam) * 8u); ++i)
		{
			const ETeam team = static_cast<ETeam>(i);
			teamTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusEntity::IterateSystemsArgsByTeamParallel), [team, &perSystemsArgsFunction]()
			{
				IterateSystemsArgRangeForTeam<SystemsArgs>(team, GetLowestTakenEntityId(), GetHighestTakenEntityId(), perSystemsArgsFunction);
			}));
		}

		UE::Tasks::Wait(teamTasks);
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
	bool				IsInRangeOfTargetEntity() const;
	bool				IsPassenger() const;
	bool				IsCarryingPassengers() const;
	bool				IsOnTeam(ETeam team) const;
	bool				IsOnSameTeamAsOtherEntity(ArgusEntity otherEntity) const;
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

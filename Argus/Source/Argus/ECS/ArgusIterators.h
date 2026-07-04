// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "Tasks/Task.h"

namespace ArgusIterators
{
	template <typename Function>
	static void IterateEntityRange(int32 fromInclusive, int32 toIncusive, Function&& perEntityFunction)
	{
		int32 currentIndex = fromInclusive;
		while (currentIndex <= toIncusive && currentIndex >= 0)
		{
			if (ArgusEntity entity = ArgusEntity::RetrieveEntity(currentIndex))
			{
				perEntityFunction(entity);
			}

			currentIndex = ArgusEntity::FindFromEntityBitArray(true, currentIndex + 1);
		}
	}

	template <typename Function>
	static void IterateEntities(Function&& perEntityFunction)
	{
		IterateEntityRange(ArgusEntity::GetLowestTakenEntityId(), ArgusEntity::GetHighestTakenEntityId(), perEntityFunction);
	}

	template <uint8 ChunkCount, typename Function>
	static void IterateEntitiesParallel(Function&& perEntityFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<ChunkCount>> chunkTasks;

		uint16 rollingBound = ArgusEntity::GetLowestTakenEntityId();
		const uint16 upperBound = ArgusEntity::GetHighestTakenEntityId();
		const uint16 difference = upperBound - rollingBound;
		const uint16 increment = difference / ChunkCount;

		for (uint8 i = 1u; i < ChunkCount; ++i)
		{
			chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusIterators::IterateEntitiesParallel), [rollingBound, increment, &perEntityFunction]()
			{
				IterateEntityRange(rollingBound, rollingBound + (increment - 1u), perEntityFunction);
			}));
			rollingBound += increment;
		}

		chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusIterators::IterateEntitiesParallel), [rollingBound, upperBound, &perEntityFunction]()
		{
			IterateEntityRange(rollingBound, upperBound, perEntityFunction);
		}));

		UE::Tasks::Wait(chunkTasks);
	}

	template <typename Function>
	static void IterateTeamEntities(Function&& perEntityFunction)
	{
		for (uint8 i = 1u; i <= NUM_TEAMS; ++i)
		{
			const ETeam team = static_cast<ETeam>(1u << (i - 1u));
			if (!ArgusEntity::IsTeamRegistered(team))
			{
				continue;
			}

			if (ArgusEntity entity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId - i))
			{
				perEntityFunction(entity);
			}
		}
	}

	template <typename Function>
	static void IterateTeamEntitiesParallel(Function&& perEntityFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<NUM_TEAMS>> teamTasks;
		for (uint8 i = 1u; i <= NUM_TEAMS; ++i)
		{
			const ETeam team = static_cast<ETeam>(1u << (i - 1u));
			if (!ArgusEntity::IsTeamRegistered(team))
			{
				continue;
			}

			if (ArgusEntity entity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId - i))
			{
				teamTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusIterators::IterateTeamEntitiesParallel), [entity, &perEntityFunction]()
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
			if (systemsArgs.PopulateArguments(ArgusEntity::RetrieveEntity(currentIndex)))
			{
				perSystemsArgsFunction(systemsArgs);
			}

			currentIndex = ArgusEntity::FindFromEntityBitArray(true, currentIndex + 1);
		}
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemsArgRangeForTeam(ETeam team, int32 fromInclusive, int32 toIncusive, Function&& perSystemsArgsFunction)
	{
		int32 currentIndex = fromInclusive;
		SystemsArgs systemsArgs = SystemsArgs();

		while (currentIndex <= toIncusive && currentIndex >= 0)
		{
			if (systemsArgs.PopulateArguments(ArgusEntity::RetrieveEntity(currentIndex)))
			{
				if (systemsArgs.m_entity.IsOnTeam(team))
				{
					perSystemsArgsFunction(systemsArgs);
				}
			}

			currentIndex = ArgusEntity::FindFromEntityBitArray(true, currentIndex + 1);
		}
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemsArgs(Function&& perSystemsArgsFunction)
	{
		IterateSystemArgRange<SystemsArgs>(ArgusEntity::GetLowestTakenEntityId(), ArgusEntity::GetHighestTakenEntityId(), perSystemsArgsFunction);
	}

	template <typename SystemsArgs, uint8 ChunkCount, typename Function>
	static void IterateSystemsArgsParallel(Function&& perSystemsArgsFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<ChunkCount>> chunkTasks;

		uint16 rollingBound = ArgusEntity::GetLowestTakenEntityId();
		const uint16 upperBound = ArgusEntity::GetHighestTakenEntityId();
		const uint16 difference = upperBound - rollingBound;
		const uint16 increment = difference / ChunkCount;

		for (uint8 i = 1u; i < ChunkCount; ++i)
		{
			chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusIterators::IterateSystemsArgsParallel), [rollingBound, increment, &perSystemsArgsFunction]()
			{
				IterateSystemArgRange<SystemsArgs>(rollingBound, rollingBound + (increment - 1u), perSystemsArgsFunction);
			}));
			rollingBound += increment;
		}

		chunkTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusIterators::IterateSystemsArgsParallel), [rollingBound, upperBound, &perSystemsArgsFunction]()
		{
			IterateSystemArgRange<SystemsArgs>(rollingBound, upperBound, perSystemsArgsFunction);
		}));

		UE::Tasks::Wait(chunkTasks);
	}

	template <typename SystemsArgs, typename Function>
	static void IterateSystemsArgsByTeamParallel(Function&& perSystemsArgsFunction)
	{
		TArray<UE::Tasks::FTask, TInlineAllocator<NUM_TEAMS>> teamTasks;
		for (uint8 i = 0u; i <= NUM_TEAMS; ++i)
		{
			const ETeam team = i > 0u ? static_cast<ETeam>(1u << (i - 1u)) : ETeam::None;
			if (!ArgusEntity::IsTeamRegistered(team))
			{
				continue;
			}

			teamTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(ArgusIterators::IterateSystemsArgsByTeamParallel), [team, &perSystemsArgsFunction]()
			{
				IterateSystemsArgRangeForTeam<SystemsArgs>(team, ArgusEntity::GetLowestTakenEntityId(), ArgusEntity::GetHighestTakenEntityId(), perSystemsArgsFunction);
			}));
		}

		UE::Tasks::Wait(teamTasks);
	}
	
	template <typename Function>
	static void IterateTeamEntitiesInBitmask(BITMASK_ETeam teamBitmask, Function&& perTeamEntityFunction)
	{
		for (uint8 i = 1u; i <= NUM_TEAMS; ++i)
		{
			const uint8 team = 1u << (i - 1u);
			if ((team & teamBitmask) > 0u)
			{
				if (ArgusEntity entity = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId - i))
				{
					perTeamEntityFunction(entity);
				}
			}
		}
	}
}
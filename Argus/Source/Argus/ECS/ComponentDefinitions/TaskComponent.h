// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

enum class BaseState : uint8
{
	None,
	SpawnedWaitingForActorTake,
	DestroyedWaitingForActorRelease
};

enum class MovementState : uint8
{
	None,
	ProcessMoveToLocationCommand,
	ProcessMoveToEntityCommand,
	MoveToLocation,
	MoveToEntity,
	FailedToFindPath
};

enum class SpawningState : uint8
{
	None,
	ProcessSpawnEntityCommand
};

enum class AbilityState : uint8
{
	None,
	ProcessCastAbility0Command,
	ProcessCastAbility1Command,
	ProcessCastAbility2Command,
	ProcessCastAbility3Command
};

struct TaskComponent
{
	ARGUS_IGNORE()
	BaseState m_baseState = BaseState::None;
	ARGUS_IGNORE()
	MovementState m_movementState = MovementState::None;
	ARGUS_IGNORE()
	SpawningState m_spawningState = SpawningState::None;
	ARGUS_IGNORE()
	AbilityState m_abilityState = AbilityState::None;
	ARGUS_IGNORE()
	uint32 m_spawnedFromArgusActorRecordId = MAX_uint32;

	bool IsExecutingMoveTask() const
	{
		return m_movementState == MovementState::MoveToLocation || m_movementState == MovementState::MoveToEntity;
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		auto baseStateName = TEXT("");
		switch (m_baseState)
		{
			case BaseState::None:
				baseStateName = ARGUS_NAMEOF(BaseState::None);
				break;
			case BaseState::SpawnedWaitingForActorTake:
				baseStateName = ARGUS_NAMEOF(BaseState::SpawnedWaitingForActorTake);
				break;
			case BaseState::DestroyedWaitingForActorRelease:
				baseStateName = ARGUS_NAMEOF(BaseState::DestroyedWaitingForActorRelease);
				break;
			default:
				break;
		}

		auto movementStateName = TEXT("");
		switch (m_movementState)
		{
			case MovementState::None:
				movementStateName = ARGUS_NAMEOF(BaseState::None);
				break;
			case MovementState::ProcessMoveToLocationCommand:
				movementStateName = ARGUS_NAMEOF(MovementState::ProcessMoveToLocationCommand);
				break;
			case MovementState::ProcessMoveToEntityCommand:
				movementStateName = ARGUS_NAMEOF(MovementState::ProcessMoveToEntityCommand);
				break;
			case MovementState::MoveToLocation:
				movementStateName = ARGUS_NAMEOF(MovementState::ProcessMoveToLocationCommand);
				break;
			case MovementState::MoveToEntity:
				movementStateName = ARGUS_NAMEOF(MovementState::ProcessMoveToEntityCommand);
				break;
			case MovementState::FailedToFindPath:
				movementStateName = ARGUS_NAMEOF(MovementState::FailedToFindPath);
				break;
			default:
				break;
		}

		auto spawningStateName = TEXT("");
		switch (m_spawningState)
		{
			case SpawningState::None:
				spawningStateName = ARGUS_NAMEOF(SpawningState::None);
				break;
			case SpawningState::ProcessSpawnEntityCommand:
				spawningStateName = ARGUS_NAMEOF(SpawningState::ProcessSpawnEntityCommand);
				break;
			default:
				break;
		}

		auto abilityStateName = TEXT("");
		switch (m_abilityState)
		{
			case AbilityState::None:
				abilityStateName = ARGUS_NAMEOF(AbilityState::None);
				break;
			case AbilityState::ProcessCastAbility0Command:
				abilityStateName = ARGUS_NAMEOF(AbilityState::ProcessCastAbility0Command);
				break;
			case AbilityState::ProcessCastAbility1Command:
				abilityStateName = ARGUS_NAMEOF(AbilityState::ProcessCastAbility0Command);
				break;
			case AbilityState::ProcessCastAbility2Command:
				abilityStateName = ARGUS_NAMEOF(AbilityState::ProcessCastAbility0Command);
				break;
			case AbilityState::ProcessCastAbility3Command:
				abilityStateName = ARGUS_NAMEOF(AbilityState::ProcessCastAbility0Command);
				break;
			default:
				break;
		}

		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %s)\n    (%s: %s)\n    (%s: %s)\n    (%s: %s)"), 
				ARGUS_NAMEOF(TaskComponent),
				ARGUS_NAMEOF(m_baseState),
				baseStateName,
				ARGUS_NAMEOF(m_movementState),
				movementStateName,
				ARGUS_NAMEOF(m_spawningState),
				spawningStateName,
				ARGUS_NAMEOF(m_abilityState),
				abilityStateName
			)
		);
	}
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

UENUM()
enum class EBaseState : uint8
{
	None,
	SpawnedWaitingForActorTake,
	DestroyedWaitingForActorRelease
};

UENUM()
enum class EMovementState : uint8
{
	None,
	ProcessMoveToLocationCommand,
	ProcessMoveToEntityCommand,
	MoveToLocation,
	MoveToEntity,
	FailedToFindPath
};

UENUM()
enum class ESpawningState : uint8
{
	None,
	ProcessSpawnEntityCommand
};

UENUM()
enum class EAbilityState : uint8
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
	EBaseState m_baseState = EBaseState::None;
	ARGUS_IGNORE()
	EMovementState m_movementState = EMovementState::None;
	ARGUS_IGNORE()
	ESpawningState m_spawningState = ESpawningState::None;
	ARGUS_IGNORE()
	EAbilityState m_abilityState = EAbilityState::None;
	ARGUS_IGNORE()
	uint32 m_spawnedFromArgusActorRecordId = MAX_uint32;

	bool IsExecutingMoveTask() const
	{
		return m_movementState == EMovementState::MoveToLocation || m_movementState == EMovementState::MoveToEntity;
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		auto baseStateName = TEXT("");
		switch (m_baseState)
		{
			case EBaseState::None:
				baseStateName = ARGUS_NAMEOF(EBaseState::None);
				break;
			case EBaseState::SpawnedWaitingForActorTake:
				baseStateName = ARGUS_NAMEOF(EBaseState::SpawnedWaitingForActorTake);
				break;
			case EBaseState::DestroyedWaitingForActorRelease:
				baseStateName = ARGUS_NAMEOF(EBaseState::DestroyedWaitingForActorRelease);
				break;
			default:
				break;
		}

		auto movementStateName = TEXT("");
		switch (m_movementState)
		{
			case EMovementState::None:
				movementStateName = ARGUS_NAMEOF(EBaseState::None);
				break;
			case EMovementState::ProcessMoveToLocationCommand:
				movementStateName = ARGUS_NAMEOF(EMovementState::ProcessMoveToLocationCommand);
				break;
			case EMovementState::ProcessMoveToEntityCommand:
				movementStateName = ARGUS_NAMEOF(EMovementState::ProcessMoveToEntityCommand);
				break;
			case EMovementState::MoveToLocation:
				movementStateName = ARGUS_NAMEOF(EMovementState::ProcessMoveToLocationCommand);
				break;
			case EMovementState::MoveToEntity:
				movementStateName = ARGUS_NAMEOF(EMovementState::ProcessMoveToEntityCommand);
				break;
			case EMovementState::FailedToFindPath:
				movementStateName = ARGUS_NAMEOF(EMovementState::FailedToFindPath);
				break;
			default:
				break;
		}

		auto spawningStateName = TEXT("");
		switch (m_spawningState)
		{
			case ESpawningState::None:
				spawningStateName = ARGUS_NAMEOF(ESpawningState::None);
				break;
			case ESpawningState::ProcessSpawnEntityCommand:
				spawningStateName = ARGUS_NAMEOF(ESpawningState::ProcessSpawnEntityCommand);
				break;
			default:
				break;
		}

		auto abilityStateName = TEXT("");
		switch (m_abilityState)
		{
			case EAbilityState::None:
				abilityStateName = ARGUS_NAMEOF(EAbilityState::None);
				break;
			case EAbilityState::ProcessCastAbility0Command:
				abilityStateName = ARGUS_NAMEOF(EAbilityState::ProcessCastAbility0Command);
				break;
			case EAbilityState::ProcessCastAbility1Command:
				abilityStateName = ARGUS_NAMEOF(EAbilityState::ProcessCastAbility0Command);
				break;
			case EAbilityState::ProcessCastAbility2Command:
				abilityStateName = ARGUS_NAMEOF(EAbilityState::ProcessCastAbility0Command);
				break;
			case EAbilityState::ProcessCastAbility3Command:
				abilityStateName = ARGUS_NAMEOF(EAbilityState::ProcessCastAbility0Command);
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
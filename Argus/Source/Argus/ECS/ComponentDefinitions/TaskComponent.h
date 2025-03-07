// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/TaskComponentStates.h"
#include "ComponentObservers/TaskComponentObservers.h"
#include "CoreMinimal.h"

struct TaskComponent
{
	ARGUS_IGNORE()
	uint32 m_spawnedFromArgusActorRecordId = 0u;
	ARGUS_IGNORE()
	BaseState m_baseState = BaseState::Alive;
	ARGUS_IGNORE()
	MovementState m_movementState = MovementState::None;
	ARGUS_IGNORE()
	SpawningState m_spawningState = SpawningState::None;
	ARGUS_IGNORE()
	AbilityState m_abilityState = AbilityState::None;
	ARGUS_IGNORE()
	ConstructionState m_constructionState = ConstructionState::None;
	ARGUS_IGNORE()
	CombatState m_combatState = CombatState::None;

	uint16 GetOwningEntityId() const;
	void Set_m_baseState(BaseState newState);

	bool IsExecutingMoveTask() const
	{
		return m_movementState == MovementState::MoveToLocation || m_movementState == MovementState::MoveToEntity;
	}

	void SetToKillState()
	{
		Set_m_baseState(BaseState::Dead);
		m_movementState = MovementState::None;
		m_spawningState = SpawningState::None;
		m_abilityState = AbilityState::None;
		m_constructionState = ConstructionState::None;
		m_combatState = CombatState::None;
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		const WIDECHAR* baseStateName = TEXT("");
		switch (m_baseState)
		{
			case BaseState::Alive:
				baseStateName = ARGUS_NAMEOF(BaseState::Alive);
				break;
			case BaseState::Dead:
				baseStateName = ARGUS_NAMEOF(BaseState::Dead);
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

		const WIDECHAR* movementStateName = TEXT("");
		switch (m_movementState)
		{
			case MovementState::None:
				movementStateName = ARGUS_NAMEOF(MovementState::None);
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

		const WIDECHAR* spawningStateName = TEXT("");
		switch (m_spawningState)
		{
			case SpawningState::None:
				spawningStateName = ARGUS_NAMEOF(SpawningState::None);
				break;
			case SpawningState::WaitingToSpawnEntity:
				spawningStateName = ARGUS_NAMEOF(SpawningState::WaitingToSpawnEntity);
				break;
			case SpawningState::SpawningEntity:
				spawningStateName = ARGUS_NAMEOF(SpawningState::SpawningEntity);
				break;
			default:
				break;
		}

		const WIDECHAR* abilityStateName = TEXT("");
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

		const WIDECHAR* constructionStateName = TEXT("");
		switch (m_constructionState)
		{
			case ConstructionState::None:
				constructionStateName = ARGUS_NAMEOF(ConstructionState::None);
				break;
			case ConstructionState::ConstructingOther:
				constructionStateName = ARGUS_NAMEOF(ConstructionState::ConstructingOther);
				break;
			case ConstructionState::BeingConstructed:
				constructionStateName = ARGUS_NAMEOF(ConstructionState::BeingConstructed);
				break;
			case ConstructionState::ConstructionFinished:
				constructionStateName = ARGUS_NAMEOF(ConstructionState::ConstructionFinished);
				break;
			default:
				break;
		}

		const WIDECHAR* combatStateName = TEXT("");
		switch (m_combatState)
		{
			case CombatState::None:
				combatStateName = ARGUS_NAMEOF(CombatState::None);
				break;
			case CombatState::Attack:
				combatStateName = ARGUS_NAMEOF(CombatState::Attack);
				break;
			default:
				break;
		}

		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %s)\n    (%s: %s)\n    (%s: %s)\n    (%s: %s)\n    (%s: %s)\n    (%s: %s)"), 
				ARGUS_NAMEOF(TaskComponent),
				ARGUS_NAMEOF(m_baseState),
				baseStateName,
				ARGUS_NAMEOF(m_movementState),
				movementStateName,
				ARGUS_NAMEOF(m_spawningState),
				spawningStateName,
				ARGUS_NAMEOF(m_abilityState),
				abilityStateName,
				ARGUS_NAMEOF(m_constructionState),
				constructionStateName,
				ARGUS_NAMEOF(m_combatState),
				combatStateName
			)
		);
	}
};
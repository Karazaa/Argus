// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/TaskComponentStates.h"
#include "ComponentObservers/TaskComponentObservers.h"
#include "CoreMinimal.h"

struct TaskComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_IGNORE()
	uint32 m_spawnedFromArgusActorRecordId = 0u;

	ARGUS_OBSERVABLE_DECLARATION(BaseState, m_baseState, BaseState::Alive)

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
};
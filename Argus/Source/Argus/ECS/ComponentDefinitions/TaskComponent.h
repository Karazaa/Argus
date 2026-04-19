// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/TaskComponentStates.h"
#include "ComponentDependencies/TeamCommanderPriorities.h"
#include "ComponentObservers/TaskComponentObservers.h"
#include "CoreMinimal.h"

struct TaskComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_NO_DATA
	uint32 m_spawnedFromArgusActorRecordId = 0u;

	ARGUS_OBSERVABLE_DECLARATION(EBaseState, m_baseState, EBaseState::Alive)

	ARGUS_NO_DATA
	EMovementState m_movementState = EMovementState::None;

	ARGUS_NO_DATA
	ESpawningState m_spawningState = ESpawningState::None;

	ARGUS_NO_DATA
	EAbilityState m_abilityState = EAbilityState::None;

	ARGUS_NO_DATA
	EConstructionState m_constructionState = EConstructionState::None;

	ARGUS_NO_DATA
	ECombatState m_combatState = ECombatState::None;

	ARGUS_NO_DATA
	EResourceExtractionState m_resourceExtractionState = EResourceExtractionState::None;

	ARGUS_OBSERVABLE_PROPERTY_DECLARATION(EFlightState, m_flightState, EFlightState::Grounded)

	ARGUS_NO_DATA
	ETeamCommanderDirective m_directiveFromTeamCommander = ETeamCommanderDirective::Count;

	bool IsExecutingMoveTask() const
	{
		return m_movementState == EMovementState::MoveToLocation || m_movementState == EMovementState::MoveToEntity;
	}

	void SetToKillState()
	{
		Set_m_baseState(EBaseState::Dead);
		m_movementState = EMovementState::None;
		m_spawningState = ESpawningState::None;
		m_abilityState = EAbilityState::None;
		m_constructionState = EConstructionState::None;
		m_combatState = ECombatState::None;
		m_resourceExtractionState = EResourceExtractionState::None;
		Set_m_flightState(EFlightState::Grounded);
	}
};
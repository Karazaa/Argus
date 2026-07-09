// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "ComponentDependencies/Teams.h"
#include "CoreMinimal.h"

struct TeamCommanderCombatDataComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numCanAttackGroundedCombatants[NUM_TEAMS];

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numCanAttackFlyingCombatants[NUM_TEAMS];

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numGroundedCombatants[NUM_TEAMS];

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numFlyingCombatants[NUM_TEAMS];

	void IncrementCanAttackGroundedCombatants(ETeam team)
	{
		m_numCanAttackGroundedCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))]++;
	}

	void IncrementCanAttackFlyingCombatants(ETeam team)
	{
		m_numCanAttackFlyingCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))]++;
	}

	void IncrementGroundedCombatants(ETeam team)
	{
		m_numGroundedCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))]++;
	}

	void IncrementFlyingCombatants(ETeam team)
	{
		m_numFlyingCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))]++;
	}

	uint16 GetNumCanAttackGroundedCombatants(ETeam team) const
	{
		return m_numCanAttackGroundedCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))];
	}

	uint16 GetNumCanAttackFlyingCombatants(ETeam team) const
	{
		return m_numCanAttackFlyingCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))];
	}

	uint16 GetNumGroundedCombatants(ETeam team) const
	{
		return m_numGroundedCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))];
	}

	uint16 GetNumFlyingCombatants(ETeam team) const
	{
		return m_numFlyingCombatants[static_cast<uint16>(FMath::FloorLog2(static_cast<uint32>(team)))];
	}

	void ClearTeamCountArrays()
	{
		for (uint8 i = 0; i < NUM_TEAMS; ++i)
		{
			m_numCanAttackGroundedCombatants[i] = 0u;
			m_numCanAttackFlyingCombatants[i] = 0u;
			m_numGroundedCombatants[i] = 0u;
			m_numFlyingCombatants[i] = 0u;
		}
	}
};

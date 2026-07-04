// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
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
};

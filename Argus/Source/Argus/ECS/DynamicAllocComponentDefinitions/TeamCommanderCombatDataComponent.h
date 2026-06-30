// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct TeamCommanderCombatDataComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numCombatantsCanAttackGrounded = 0u;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numCombatantsCanAttackFlying = 0u;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numEnemyCombatantsGrounded = 0u;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	uint16 m_numEnemyCombatantsFlying = 0u;
};

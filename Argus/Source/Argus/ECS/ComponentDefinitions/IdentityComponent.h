// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "IdentityComponent.generated.h"

class UFactionRecord;

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETeam : uint8
{
	None = 0,
	TeamA = 1 << 0,
	TeamB = 1 << 1,
	TeamC = 1 << 2,
	TeamD = 1 << 3,
	TeamE = 1 << 4,
	TeamF = 1 << 5,
	TeamG = 1 << 6,
	TeamH = 1 << 7
};
ENUM_CLASS_FLAGS(ETeam);

struct IdentityComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_STATIC_DATA(UFactionRecord)
	uint32 m_factionId = 0u;

	ARGUS_IGNORE()
	ETeam m_team = ETeam::None;

	ARGUS_IGNORE()
	uint8 m_allies = 0u;

	ARGUS_IGNORE()
	uint8 m_enemies = 0u;

	void AddEnemyTeam(ETeam enemyTeam)
	{
		if (enemyTeam == m_team)
		{
			return;
		}

		m_allies &= ~((uint8)enemyTeam);
		m_enemies |= ((uint8)enemyTeam);
	}

	void AddAllyTeam(ETeam allyTeam)
	{
		m_enemies &= ~((uint8)allyTeam);
		m_allies |= ((uint8)allyTeam);
	}

	bool IsInTeamMask(uint8 teamMask) const
	{
		return teamMask & ((uint8)m_team);
	}
};
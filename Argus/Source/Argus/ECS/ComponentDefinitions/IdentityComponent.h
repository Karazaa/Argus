// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

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
	ARGUS_COMPONENT_SHARED

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

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		auto teamName = TEXT("");
		switch (m_team)
		{
			case ETeam::None:
				teamName = ARGUS_NAMEOF(ETeam::None);
				break;
			case ETeam::TeamA:
				teamName = ARGUS_NAMEOF(ETeam::TeamA);
				break;
			case ETeam::TeamB:
				teamName = ARGUS_NAMEOF(ETeam::TeamB);
				break;
			case ETeam::TeamC:
				teamName = ARGUS_NAMEOF(ETeam::TeamC);
				break;
			case ETeam::TeamD:
				teamName = ARGUS_NAMEOF(ETeam::TeamD);
				break;
			case ETeam::TeamE:
				teamName = ARGUS_NAMEOF(ETeam::TeamE);
				break;
			case ETeam::TeamF:
				teamName = ARGUS_NAMEOF(ETeam::TeamF);
				break;
			case ETeam::TeamG:
				teamName = ARGUS_NAMEOF(ETeam::TeamG);
				break;
			case ETeam::TeamH:
				teamName = ARGUS_NAMEOF(ETeam::TeamH);
				break;
		}

		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (%s: %S)"), 
				ARGUS_NAMEOF(IdentityComponent),
				ARGUS_NAMEOF(teamName),
				teamName
			)
		);
	}
};
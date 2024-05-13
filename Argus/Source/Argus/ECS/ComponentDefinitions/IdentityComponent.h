// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUtil.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFaction : uint8
{
	None = 0,
	FactionA = 1 << 0,
	FactionB = 1 << 1,
	FactionC = 1 << 2,
	FactionD = 1 << 3,
	FactionE = 1 << 4,
	FactionF = 1 << 5,
	FactionG = 1 << 7
};
ENUM_CLASS_FLAGS(EFaction);

struct IdentityComponent
{
	EFaction m_faction = EFaction::None;

	ARGUS_PROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = EFaction))
	uint8 m_allies = 0u;

	ARGUS_PROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = EFaction))
	uint8 m_enemies = 0u;

	void AddEnemyFaction(EFaction enemyFaction)
	{
		if (enemyFaction == m_faction)
		{
			return;
		}

		m_allies &= ~((uint8)enemyFaction);
		m_enemies |= ((uint8)enemyFaction);
	}

	void AddAllyFaction(EFaction allyFaction)
	{
		m_enemies &= ~((uint8)allyFaction);
		m_allies |= ((uint8)allyFaction);
	}

	bool IsInFactionMask(uint8 factionMask) const
	{
		return factionMask & ((uint8)m_faction);
	}
};
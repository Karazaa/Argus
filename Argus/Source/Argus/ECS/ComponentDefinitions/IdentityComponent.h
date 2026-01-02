// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/Teams.h"

class UFactionRecord;

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

	ARGUS_IGNORE()
	uint8 m_seenBy = 0u;

	ARGUS_IGNORE()
	uint8 m_everSeenBy = 0u;

	void AddEnemyTeam(ETeam enemyTeam)
	{
		if (enemyTeam == m_team)
		{
			return;
		}

		m_allies &= ~(static_cast<uint8>(enemyTeam));
		m_enemies |= (static_cast<uint8>(enemyTeam));
	}

	void AddAllyTeam(ETeam allyTeam)
	{
		m_enemies &= ~(static_cast<uint8>(allyTeam));
		m_allies |= (static_cast<uint8>(allyTeam));
	}

	bool IsInTeamMask(uint8 teamMask) const
	{
		return teamMask & (static_cast<uint8>(m_team));
	}

	void AddSeenBy(ETeam seeingTeam)
	{
		m_seenBy |= (static_cast<uint8>(seeingTeam));
		m_everSeenBy |= m_seenBy;
	}

	void ClearSeenBy()
	{
		m_seenBy = static_cast<uint8>(m_team);
	}

	bool IsSeenBy(ETeam team) const
	{
		return m_seenBy & (static_cast<uint8>(team));
	}

	bool WasEverSeenBy(ETeam team) const
	{
		return m_everSeenBy & (static_cast<uint8>(team));
	}

	bool IsSeenByAllies() const 
	{
		return m_seenBy & m_allies;
	}

	bool IsSeenByEnemies() const
	{
		return m_seenBy & m_enemies;
	}
};
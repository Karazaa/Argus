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
	BITMASK_ETeam m_allies = 0u;

	ARGUS_IGNORE()
	BITMASK_ETeam m_enemies = 0u;

	ARGUS_IGNORE()
	BITMASK_ETeam m_seenBy = 0u;

	ARGUS_IGNORE()
	BITMASK_ETeam m_everSeenBy = 0u;

	void AddEnemyTeam(ETeam enemyTeam)
	{
		if (enemyTeam == m_team)
		{
			return;
		}

		m_allies &= ~(static_cast<BITMASK_ETeam>(enemyTeam));
		m_enemies |= (static_cast<BITMASK_ETeam>(enemyTeam));
	}

	void AddAllyTeam(ETeam allyTeam)
	{
		m_enemies &= ~(static_cast<BITMASK_ETeam>(allyTeam));
		m_allies |= (static_cast<BITMASK_ETeam>(allyTeam));
	}

	bool IsInTeamMask(BITMASK_ETeam teamMask) const
	{
		return teamMask & (static_cast<BITMASK_ETeam>(m_team));
	}

	void AddSeenBy(ETeam seeingTeam)
	{
		m_seenBy |= (static_cast<BITMASK_ETeam>(seeingTeam));
		m_everSeenBy |= m_seenBy;
	}

	void ClearSeenBy()
	{
		m_seenBy = static_cast<BITMASK_ETeam>(m_team);
	}

	bool IsSeenBy(ETeam team) const
	{
		return m_seenBy & (static_cast<BITMASK_ETeam>(team));
	}

	bool WasEverSeenBy(ETeam team) const
	{
		return m_everSeenBy & (static_cast<BITMASK_ETeam>(team));
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
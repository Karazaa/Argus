// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/Teams.h"

class UFactionRecord;

struct IdentityComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_COMP_STATIC_DATA(UFactionRecord)
	uint32 m_factionId = 0u;

	ARGUS_COMP_NO_DATA
	ETeam m_team = ETeam::None;

	ARGUS_COMP_NO_DATA
	BITMASK_ETeam m_seenBy = 0u;

	ARGUS_COMP_NO_DATA
	BITMASK_ETeam m_everSeenBy = 0u;

	bool IsInTeamMask(BITMASK_ETeam teamMask) const
	{
		return TeamUtils::IsInTeamMask(m_team, teamMask);
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
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"

struct NearbyEntitiesComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	ArgusEntityKDTreeRangeOutput m_nearbyEntities;

	ARGUS_IGNORE()
	ArgusEntityKDTreeRangeOutput m_nearbyFlyingEntities;

	const ArgusEntityKDTreeRangeOutput& GetNearbyEntities(bool shouldGetFlying) const
	{
		return shouldGetFlying ? m_nearbyFlyingEntities : m_nearbyEntities;
	}

	template <typename Function>
	void IterateSeenEntityIds(bool iterateGrounded, bool iterateFlying, Function&& perEntityIdFunction) const
	{
		if (iterateGrounded)
		{
			for (int32 i = 0; i < m_nearbyEntities.GetEntityIdsInSightRange().Num(); ++i)
			{
				if (perEntityIdFunction(m_nearbyEntities.GetEntityIdsInSightRange()[i]))
				{
					return;
				}
			}
		}

		if (iterateFlying)
		{
			for (int32 i = 0; i < m_nearbyFlyingEntities.GetEntityIdsInSightRange().Num(); ++i)
			{
				if (perEntityIdFunction(m_nearbyFlyingEntities.GetEntityIdsInSightRange()[i]))
				{
					return;
				}
			}
		}
	}
};

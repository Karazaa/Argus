// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusEntityKDTree.h"
#include "CoreMinimal.h"

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
};

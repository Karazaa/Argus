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
};

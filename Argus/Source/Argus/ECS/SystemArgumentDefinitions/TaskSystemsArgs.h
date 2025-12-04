// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TaskSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;
	NearbyEntitiesComponent* m_nearbyEntitiesComponent = nullptr;

	ARGUS_GET_BUT_SKIP()
	NavigationComponent* m_navigationComponent = nullptr;
};

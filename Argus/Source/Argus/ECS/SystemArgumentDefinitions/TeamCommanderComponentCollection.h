// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TeamCommanderComponentCollection
{
	ARGUS_SYSTEM_ARGS_SHARED;

	ResourceComponent* m_currentResourcesComponent = nullptr;
	TeamCommanderComponent* m_baseComponent = nullptr;
	TeamCommanderResourceDataComponent* m_resourceDataComponent = nullptr;
	TeamCommanderCombatDataComponent* m_combatDataComponent = nullptr;
};

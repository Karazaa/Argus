// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

struct CombatSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	CombatComponent* m_combatComponent = nullptr;
	IdentityComponent* m_identityComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;
	TaskComponent* m_taskComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;
};
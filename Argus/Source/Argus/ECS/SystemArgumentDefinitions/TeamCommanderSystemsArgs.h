// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TeamCommanderSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;
	IdentityComponent* m_identityComponent = nullptr;

	ARGUS_GET_BUT_SKIP()
	TransformComponent* m_transformComponent = nullptr;

	ARGUS_GET_BUT_SKIP()
	TargetingComponent* m_targetingComponent = nullptr;
};

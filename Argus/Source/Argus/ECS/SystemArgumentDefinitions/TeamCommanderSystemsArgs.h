// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TeamCommanderSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	IdentityComponent* m_identityComponent = nullptr;
	TaskComponent* m_taskComponent = nullptr;
	
	ARGUS_GET_BUT_SKIP()
	ResourceComponent* m_resourceComponent = nullptr;

	ARGUS_GET_BUT_SKIP()
	TargetingComponent* m_targetingComponent = nullptr;

	ARGUS_GET_BUT_SKIP()
	TransformComponent* m_transformComponent = nullptr;
};

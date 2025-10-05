// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct FlockingSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;
	AvoidanceGroupingComponent* m_avoidanceGroupingComponent = nullptr;
	FlockingComponent* m_flockingComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;
};

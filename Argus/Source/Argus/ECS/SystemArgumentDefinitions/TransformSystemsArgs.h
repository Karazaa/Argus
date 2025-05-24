// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TransformSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;
	VelocityComponent* m_velocityComponent = nullptr;
	NavigationComponent* m_navigationComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;
};
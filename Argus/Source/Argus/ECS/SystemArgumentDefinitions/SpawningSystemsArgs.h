// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

struct SpawningSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	SpawningComponent* m_spawningComponent = nullptr;
	TaskComponent* m_taskComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;
};
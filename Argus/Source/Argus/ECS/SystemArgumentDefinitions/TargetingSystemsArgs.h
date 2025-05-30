// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TargetingSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TargetingComponent* m_targetingComponent = nullptr;
	const TransformComponent* m_transformComponent = nullptr;
};
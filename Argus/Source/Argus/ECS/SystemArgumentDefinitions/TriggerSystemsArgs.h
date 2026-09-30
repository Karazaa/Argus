// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TriggerSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TriggerComponent* m_triggerComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;

	ARGUS_SYSARG_UNCHECKED_GET
	RelativeCircleComponent* m_relativeCircleComponent = nullptr;

	ARGUS_SYSARG_UNCHECKED_GET
	RelativePolygonComponent* m_relativePolygonComponent = nullptr;
};

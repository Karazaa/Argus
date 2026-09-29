// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct TriggerSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	RelativePolygonComponent* m_relativePolygonComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;
};

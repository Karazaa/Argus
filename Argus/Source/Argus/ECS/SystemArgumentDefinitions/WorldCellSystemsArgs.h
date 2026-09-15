// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct WorldCellSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TransformComponent* m_transformComponent = nullptr;
	WorldCellComponent* m_worldCellComponent = nullptr;
};

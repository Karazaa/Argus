// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct FogOfWarSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	FogOfWarLocationComponent* m_fogOfWarLocationComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;
	TaskComponent* m_taskComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;

	ARGUS_SYSARG_UNCHECKED_GET
	NearbyObstaclesComponent* m_nearbyObstaclesComponent = nullptr;
};

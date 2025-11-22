// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"

struct DecalComponent
{
	ARGUS_COMPONENT_SHARED;

	float m_lifetimeSeconds = 1.0f;

	ARGUS_IGNORE()
	TimerHandle m_lifetimeTimer;
};

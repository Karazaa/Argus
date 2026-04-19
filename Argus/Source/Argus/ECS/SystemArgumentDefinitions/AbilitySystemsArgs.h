// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct AbilitySystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	AbilityComponent* m_abilityComponent = nullptr;
	TaskComponent* m_taskComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;

	ARGUS_NO_DATA
	ReticleComponent* m_reticleComponent = nullptr;
};
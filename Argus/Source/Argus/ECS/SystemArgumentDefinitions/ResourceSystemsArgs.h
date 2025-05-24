// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

struct ResourceSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;
	ResourceComponent* m_resourceComponent = nullptr;
	ResourceExtractionComponent* m_resourceExtractionComponent = nullptr;
	TargetingComponent* m_targetingComponent = nullptr;
};
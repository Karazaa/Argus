// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

struct ConstructionSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;

	ARGUS_SYSARG_UNCHECKED_GET
	TargetingComponent* m_targetingComponent = nullptr;

	ARGUS_SYSARG_UNCHECKED_GET
	ConstructionComponent* m_constructionComponent = nullptr;
};
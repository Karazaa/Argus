// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

struct ConstructionSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;
	ConstructionComponent* m_constructionComponent = nullptr;
};
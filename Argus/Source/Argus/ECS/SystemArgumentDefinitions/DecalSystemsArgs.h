// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "ArgusMacros.h"

struct DecalSystemsArgs
{
	ARGUS_SYSTEM_ARGS_SHARED;

	TaskComponent* m_taskComponent = nullptr;
	DecalComponent* m_decalComponent = nullptr;
	TimerComponent* m_timerComponent = nullptr;
	TransformComponent* m_transformComponent = nullptr;
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusQueue.h"
#include "ComponentDependencies/SpawnEntityInfo.h"
#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"

class UArgusActorRecord;

struct SpawningComponent
{
	ARGUS_COMPONENT_SHARED

	float m_spawningRadius = 100.0f;

	uint8 m_maximumQueueSize = 10u;

	ARGUS_IGNORE()
	uint8 m_currentQueueSize = 0u;

	ARGUS_IGNORE()
	TimerHandle m_spawnTimerHandle;

	ARGUS_IGNORE()
	ArgusQueue<SpawnEntityInfo> m_spawnQueue;
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/ArgusDeque.h"
#include "ComponentDependencies/SpawnEntityInfo.h"
#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"

class UArgusActorRecord;

struct SpawningComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_COMP_NO_DATA
	ArgusDeque<SpawnEntityInfo, ArgusContainerAllocator<10> > m_spawnQueue;

	float m_spawningRadius = 100.0f;

	uint8 m_maximumQueueSize = 10u;

	ARGUS_COMP_NO_DATA
	TOptional<uint8> m_spawnQueueIndexToCancel;

	ARGUS_COMP_NO_DATA
	TimerHandle m_spawnTimerHandle;
};
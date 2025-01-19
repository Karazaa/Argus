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
	ARGUS_STATIC_DATA(UArgusActorRecord)
	uint32 m_argusActorRecordId = 0u;

	float m_spawningRadius = 100.0f;

	uint8 m_maximumQueueSize = 10u;

	ARGUS_IGNORE()
	TimerHandle m_spawnTimerHandle;

	ARGUS_IGNORE()
	ArgusQueue<SpawnEntityInfo> m_spawnQueue;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s]\n    (%s: %d)\n    (%s: %f)"),
				ARGUS_NAMEOF(SpawningComponent),
				ARGUS_NAMEOF(m_argusActorRecordId),
				m_argusActorRecordId,
				ARGUS_NAMEOF(m_spawningRadius),
				m_spawningRadius
			)
		);
	}
};
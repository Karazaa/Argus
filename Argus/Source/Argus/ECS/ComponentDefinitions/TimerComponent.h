// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "ComponentDependencies/Timer.h"
#include <vector>

struct TimerComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_IGNORE()
	TArray<Timer> m_timers;

	Timer* GetTimerFromHandle(const TimerHandle* timerHandle)
	{
		if (!timerHandle)
		{
			ARGUS_LOG
			(
				ArgusECSLog,
				Error,
				TEXT("[%s] Passed in %s was null."),
				ARGUS_FUNCNAME,
				ARGUS_NAMEOF(TimerHandle*)
			)
			return nullptr;
		}

		const uint8 timerIndex = timerHandle->GetTimerIndex();
		if (timerIndex == UINT8_MAX)
		{
			return nullptr;
		}

		if (m_timers.Num() <= timerHandle->GetTimerIndex())
		{
			return nullptr;
		}

		return &m_timers[timerIndex];
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s]"),
				ARGUS_NAMEOF(TimerComponent)
			)
		);

		for (int32 i = 0; i < m_timers.Num(); ++i)
		{
			debugStringToAppendTo.Append
			(
				FString::Printf
				(
					TEXT("\n    (%d: %f)"),
					i,
					m_timers[i].m_timeRemainingSeconds
				)
			);
		}
	}
};
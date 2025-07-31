// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusArrayAllocator.h"
#include "ArgusLogging.h"
#include "CoreMinimal.h"
#include "ComponentDependencies/Timer.h"
#include <vector>

struct TimerComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	TArray<Timer, ArgusContainerAllocator<2> > m_timers;

	Timer* GetTimerFromHandle(const TimerHandle* timerHandle)
	{
		ARGUS_RETURN_ON_NULL_POINTER(timerHandle, ArgusECSLog);

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
};
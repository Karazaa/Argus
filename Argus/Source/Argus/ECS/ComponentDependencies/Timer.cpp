// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Timer.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ComponentDefinitions/TimerComponent.h"

void TimerHandle::StartTimer(ArgusEntity& entityWithTimer, float seconds)
{
	TimerComponent* timerComponent = GetTimerComponentForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timerComponent)
	{
		return;
	}

	for (uint8 i = 0u; i < timerComponent->m_timers.size(); ++i)
	{
		if (timerComponent->m_timers[i].m_timerState == TimerState::NotSet)
		{
			timerComponent->m_timers[i].m_timerState = TimerState::Ticking;
			timerComponent->m_timers[i].m_timeRemainingSeconds = seconds;
			m_timerIndex = i;
			return;
		}
	}
}

void TimerHandle::FinishTimerHandling(ArgusEntity& entityWithTimer)
{
	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return;
	}

	if (timer->m_timerState != TimerState::Completed)
	{
		return;
	}

	timer->m_timeRemainingSeconds = 0.0f;
	timer->m_timerState = TimerState::NotSet;
	m_timerIndex = UINT8_MAX;
}

void TimerHandle::CancelTimer(ArgusEntity& entityWithTimer)
{
	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return;
	}

	timer->m_timeRemainingSeconds = 0.0f;
	timer->m_timerState = TimerState::NotSet;
	m_timerIndex = UINT8_MAX;
}

bool TimerHandle::IsTimerComplete(ArgusEntity& entityWithTimer) const
{
	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return false;
	}

	return timer->m_timerState == TimerState::Completed;
}

TimerComponent* TimerHandle::GetTimerComponentForEntity(ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const
{
	if (!entityWithTimer)
	{
		ARGUS_LOG
		(
			ArgusECSLog, 
			Error, 
			TEXT("[%s] Passed in %s is invalid when trying to retrieve %s."), 
			functionName, 
			ARGUS_NAMEOF(ArgusEntity), 
			ARGUS_NAMEOF(TimerComponent)
		);
		return nullptr;
	}

	TimerComponent* timerComponent = entityWithTimer.GetComponent<TimerComponent>();
	if (!timerComponent)
	{
		ARGUS_LOG
		(
			ArgusECSLog, 
			Error, 
			TEXT("[%s] Could not find a %s associated with passed in %s (%d)."), 
			functionName, 
			ARGUS_NAMEOF(TimerComponent), 
			ARGUS_NAMEOF(ArgusEntity), 
			entityWithTimer.GetId()
		);
		return nullptr;
	}

	return timerComponent;
}

Timer* TimerHandle::GetTimerForEntity(ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const
{
	TimerComponent* timerComponent = GetTimerComponentForEntity(entityWithTimer, functionName);
	if (!timerComponent)
	{
		return nullptr;
	}

	Timer* timer = timerComponent->GetTimerFromHandle(this);
	if (!timer)
	{
		ARGUS_LOG
		(
			ArgusECSLog,
			Error,
			TEXT("[%s] Could not retrieve a %s from %s associated with passed in %s (%d)."),
			functionName,
			ARGUS_NAMEOF(Timer*),
			ARGUS_NAMEOF(TimerComponent),
			ARGUS_NAMEOF(ArgusEntity),
			entityWithTimer.GetId()
		);
		return nullptr;
	}

	return timer;
}
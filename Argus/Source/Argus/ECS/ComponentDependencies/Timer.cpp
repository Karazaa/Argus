// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Timer.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ComponentDefinitions/TimerComponent.h"

void TimerHandle::StartTimer(float seconds)
{
	ArgusEntity entity = ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this));
	StartTimer(entity, seconds);
}

void TimerHandle::StartTimer(const ArgusEntity& entityWithTimer, float seconds)
{
	ARGUS_MEMORY_TRACE(ArgusTimerSystems);

	if (m_timerIndex != UINT8_MAX)
	{
		ARGUS_LOG
		(
			ArgusECSLog,
			Error,
			TEXT("[%s] Attempting to start a timer on a %s that is already assigned."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle)
		);
		return;
	}

	TimerComponent* timerComponent = GetTimerComponentForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timerComponent)
	{
		return;
	}

	for (int32 i = 0; i < timerComponent->m_timers.Num(); ++i)
	{
		if (timerComponent->m_timers[i].m_timerState == TimerState::NotSet)
		{
			timerComponent->m_timers[i].m_timerState = TimerState::Ticking;
			timerComponent->m_timers[i].m_initialDurationSeconds = seconds;
			timerComponent->m_timers[i].m_timeRemainingSeconds = seconds;
			m_timerIndex = static_cast<uint8>(i);
			return;
		}
	}

	Timer timerToAdd;
	timerToAdd.m_timerState = TimerState::Ticking;
	timerToAdd.m_initialDurationSeconds = seconds;
	timerToAdd.m_timeRemainingSeconds = seconds;
	timerComponent->m_timers.Add(timerToAdd);
	m_timerIndex = static_cast<uint8>(timerComponent->m_timers.Num() - 1);
}

void TimerHandle::FinishTimerHandling()
{
	ArgusEntity entity = ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this));
	FinishTimerHandling(entity);
}

void TimerHandle::FinishTimerHandling(const ArgusEntity& entityWithTimer)
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

	timer->m_initialDurationSeconds = 0.0f;
	timer->m_timeRemainingSeconds = 0.0f;
	timer->m_timerState = TimerState::NotSet;
	m_timerIndex = UINT8_MAX;
}

void TimerHandle::CancelTimer()
{
	CancelTimer(ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this)));
}

void TimerHandle::CancelTimer(const ArgusEntity& entityWithTimer)
{
	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		m_timerIndex = UINT8_MAX;
		return;
	}

	timer->m_initialDurationSeconds = 0.0f;
	timer->m_timeRemainingSeconds = 0.0f;
	timer->m_timerState = TimerState::NotSet;
	m_timerIndex = UINT8_MAX;
}

float TimerHandle::GetTimeRemaining() const
{
	return GetTimeRemaining(ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this)));
}

float TimerHandle::GetTimeRemaining(const ArgusEntity& entityWithTimer) const
{
	if (m_timerIndex == UINT8_MAX)
	{
		return 0.0f;
	}

	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return 0.0f;
	}

	if (timer->m_initialDurationSeconds == 0.0f)
	{
		return 0.0f;
	}

	return timer->m_timeRemainingSeconds;
}

float TimerHandle::GetTimeElapsedProportion() const
{
	return GetTimeElapsedProportion(ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this)));
}

float TimerHandle::GetTimeElapsedProportion(const ArgusEntity& entityWithTimer) const
{
	if (m_timerIndex == UINT8_MAX)
	{
		return 0.0f;
	}

	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return 0.0f;
	}

	if (timer->m_initialDurationSeconds == 0.0f)
	{
		return 0.0f;
	}

	return 1.0f - (timer->m_timeRemainingSeconds / timer->m_initialDurationSeconds);
}

bool TimerHandle::IsTimerTicking() const
{
	return IsTimerTicking(ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this)));
}

bool TimerHandle::IsTimerTicking(const ArgusEntity& entityWithTimer) const
{
	if (m_timerIndex == UINT8_MAX)
	{
		return false;
	}

	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return false;
	}

	return timer->m_timerState == TimerState::Ticking;
}

bool TimerHandle::IsTimerComplete(const ArgusEntity& entityWithTimer) const
{
	if (m_timerIndex == UINT8_MAX)
	{
		return false;
	}

	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return false;
	}

	return timer->m_timerState == TimerState::Completed;
}

bool TimerHandle::IsTimerComplete() const
{
	return IsTimerComplete(ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this)));
}

bool TimerHandle::WasTimerSet() const
{
	return WasTimerSet(ArgusEntity::RetrieveEntity(ArgusComponentRegistry::GetOwningEntityIdForComponentMember(this)));
}

bool TimerHandle::WasTimerSet(const ArgusEntity& entityWithTimer) const
{
	if (m_timerIndex == UINT8_MAX)
	{
		return false;
	}

	Timer* timer = GetTimerForEntity(entityWithTimer, ARGUS_FUNCNAME);
	if (!timer)
	{
		return false;
	}

	return timer->m_timerState != TimerState::NotSet;
}

TimerComponent* TimerHandle::GetTimerComponentForEntity(const ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const
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

Timer* TimerHandle::GetTimerForEntity(const ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const
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
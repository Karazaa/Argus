// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TimerSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"

void TimerSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TimerSystems::RunSystems);

	ArgusEntity::IterateEntities([deltaTime](ArgusEntity entity) 
	{
		TimerComponent* timerComponent = entity.GetComponent<TimerComponent>();
		if (!timerComponent)
		{
			return;
		}

		AdvaceTimers(deltaTime, timerComponent);
	});

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	TimerComponent* singletonTimerComponent = singletonEntity.GetComponent<TimerComponent>();
	if (!singletonTimerComponent)
	{
		return;
	}

	AdvaceTimers(deltaTime, singletonTimerComponent);
}

void TimerSystems::AdvaceTimers(float deltaTime, TimerComponent* timerComponent)
{
	ARGUS_TRACE(TimerSystems::AdvaceTimers);

	if (!timerComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in %s was null."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TimerComponent*));
		return;
	}

	for (int32 i = 0; i < timerComponent->m_timers.Num(); ++i)
	{
		if (timerComponent->m_timers[i].m_timerState != TimerState::Ticking)
		{
			continue;
		}

		timerComponent->m_timers[i].m_timeRemainingSeconds -= deltaTime;
		if (timerComponent->m_timers[i].m_timeRemainingSeconds <= 0.0f)
		{
			timerComponent->m_timers[i].m_timeRemainingSeconds = 0.0f;
			timerComponent->m_timers[i].m_timerState = TimerState::Completed;
		}
	}
}

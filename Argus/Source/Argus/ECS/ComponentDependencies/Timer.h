// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class ArgusEntity;
struct TimerComponent;

enum class TimerState : uint8
{
	NotSet,
	Ticking,
	Completed
};

struct Timer
{
	float m_timeRemainingSeconds = 0.0f;
	float m_initialDurationSeconds = 0.0f;
	TimerState m_timerState = TimerState::NotSet;
};

class TimerHandle
{
public:
	void StartTimer(float seconds);
	void StartTimer(ArgusEntity entityWithTimer, float seconds);
	void FinishTimerHandling();
	void FinishTimerHandling(ArgusEntity entityWithTimer);
	void CancelTimer();
	void CancelTimer(ArgusEntity entityWithTimer);

	float GetTimeRemaining() const;
	float GetTimeRemaining(ArgusEntity entityWithTimer) const;
	float GetTimeElapsedProportion() const;
	float GetTimeElapsedProportion(ArgusEntity entityWithTimer) const;

	bool IsTimerTicking() const;
	bool IsTimerTicking(ArgusEntity entityWithTimer) const;
	bool IsTimerComplete() const;
	bool IsTimerComplete(ArgusEntity entityWithTimer) const;
	bool WasTimerSet() const;
	bool WasTimerSet(ArgusEntity entityWithTimer) const;

	uint8 GetTimerIndex() const { return m_timerIndex; }

	void Reset() { m_timerIndex = UINT8_MAX; }

private:
	TimerComponent* GetTimerComponentForEntity(ArgusEntity entityWithTimer, const WIDECHAR* functionName) const;
	Timer* GetTimerForEntity(ArgusEntity entityWithTimer, const WIDECHAR* functionName) const;

	uint8 m_timerIndex = UINT8_MAX;
};
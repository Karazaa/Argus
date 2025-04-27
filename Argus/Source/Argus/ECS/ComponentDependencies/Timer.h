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
	void StartTimer(const ArgusEntity& entityWithTimer, float seconds);
	void FinishTimerHandling();
	void FinishTimerHandling(const ArgusEntity& entityWithTimer);
	void CancelTimer();
	void CancelTimer(const ArgusEntity& entityWithTimer);

	float GetTimeRemaining(const ArgusEntity& entityWithTimer) const;
	float GetTimeElapsedProportion(const ArgusEntity& entityWithTimer) const;

	bool IsTimerTicking(const ArgusEntity& entityWithTimer) const;
	bool IsTimerComplete(const ArgusEntity& entityWithTimer) const;

	uint8 GetTimerIndex() const { return m_timerIndex; }

private:
	TimerComponent* GetTimerComponentForEntity(const ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const;
	Timer* GetTimerForEntity(const ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const;

	uint8 m_timerIndex = UINT8_MAX;
};
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
	TimerState m_timerState = TimerState::NotSet;
};

class TimerHandle
{
public:
	void StartTimer(ArgusEntity& entityWithTimer, float seconds);
	void FinishTimerHandling(ArgusEntity& entityWithTimer);
	void CancelTimer(ArgusEntity& entityWithTimer);
	void ResetTimerIndex() { m_timerIndex = UINT8_MAX; }

	bool IsTimerComplete(ArgusEntity& entityWithTimer) const;

	uint8 GetTimerIndex() const { return m_timerIndex; }

private:
	TimerComponent* GetTimerComponentForEntity(ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const;
	Timer* GetTimerForEntity(ArgusEntity& entityWithTimer, const WIDECHAR* functionName) const;

	uint8 m_timerIndex = UINT8_MAX;
};
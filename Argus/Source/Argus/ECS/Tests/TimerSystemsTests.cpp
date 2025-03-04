// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "Misc/AutomationTest.h"
#include "Systems/TimerSystems.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TimerSystemsAdvanceTimersTest, "Argus.ECS.Systems.TimerSystems.AdvanceTimers", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TimerSystemsAdvanceTimersTest::RunTest(const FString& Parameters)
{
	const float deltaTime = 1.0f;
	const float timer0expectedDuration = 1.0f;
	const float timer1expectedDuration = 2.0f;
	ArgusTesting::StartArgusTest();

#pragma region Test that passing in an invalid TimerComponent errors
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Passed in %s was null."),
			ARGUS_NAMEOF(TimerComponent*)
		)
	);
#pragma endregion

	TimerSystems::AdvaceTimers(deltaTime, nullptr);

	ArgusEntity timerEntity = ArgusEntity::CreateEntity();
	TimerComponent* timerComponent = timerEntity.AddComponent<TimerComponent>();
	if (!timerComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	// Test that this doesn't cause an error when there are no timers.
	TimerSystems::AdvaceTimers(deltaTime, timerComponent);

	Timer timer0, timer1, timer2;
	timer0.m_timerState = TimerState::Ticking;
	timer1.m_timerState = TimerState::Ticking;
	timer2.m_timerState = TimerState::NotSet;
	timer0.m_timeRemainingSeconds = timer0expectedDuration;
	timer1.m_timeRemainingSeconds = timer1expectedDuration;
	timerComponent->m_timers.Add(timer0);
	timerComponent->m_timers.Add(timer1);
	timerComponent->m_timers.Add(timer2);

	TimerSystems::AdvaceTimers(deltaTime, timerComponent);

#pragma region Test timer states after advancing 1 second
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Checking state of timers after calling %s with a time step of %f seconds."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerSystems::AdvaceTimers),
			deltaTime
		),
		(timerComponent->m_timers[0].m_timerState == TimerState::Completed) &&
		(timerComponent->m_timers[1].m_timerState == TimerState::Ticking) &&
		(timerComponent->m_timers[2].m_timerState == TimerState::NotSet) && 
		(timerComponent->m_timers[1].m_timeRemainingSeconds == (timer1expectedDuration - deltaTime))
	);
#pragma endregion

	TimerSystems::AdvaceTimers(deltaTime, timerComponent);

#pragma region Test timer states after advancing 2 second
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Checking state of timers after calling %s with a time step of %f seconds."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerSystems::AdvaceTimers),
			(deltaTime * 2.0f)
		),
		(timerComponent->m_timers[0].m_timerState == TimerState::Completed) &&
		(timerComponent->m_timers[1].m_timerState == TimerState::Completed) &&
		(timerComponent->m_timers[2].m_timerState == TimerState::NotSet)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS
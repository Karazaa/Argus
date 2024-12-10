// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntity.h"
#include "ArgusTesting.h"
#include "ComponentDependencies/Timer.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TimerHandleStartTimerTest, "Argus.ECS.TimerHandle.StartTimer", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TimerHandleStartTimerTest::RunTest(const FString& Parameters)
{
	const float expectedTimerDurationSeconds = 10.0f;

	ArgusTesting::StartArgusTest();

	TimerHandle timerHandle;

#pragma region Test passing in invalid ArgusEntity to StartTimer
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Passed in %s is invalid when trying to retrieve %s."),
			ARGUS_NAMEOF(ArgusEntity),
			ARGUS_NAMEOF(TimerComponent)
		)
	);
#pragma endregion

	timerHandle.StartTimer(ArgusEntity::k_emptyEntity, expectedTimerDurationSeconds);
	ArgusEntity timerEntity = ArgusEntity::CreateEntity();

#pragma region Test starting a timer with no timer component
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Could not find a %s associated with passed in %s (%d)."),
			ARGUS_NAMEOF(TimerComponent),
			ARGUS_NAMEOF(ArgusEntity),
			timerEntity.GetId()
		)
	);
#pragma endregion

	timerHandle.StartTimer(timerEntity, expectedTimerDurationSeconds);
	TimerComponent* timerComponent = timerEntity.AddComponent<TimerComponent>();
	if (!timerComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

#pragma region Test no timers by default
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Testing that a new %s would not have any %s by default."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(TimerComponent),
			ARGUS_NAMEOF(Timer)
		),
		timerComponent->m_timers.empty()
	);
#pragma endregion

	timerHandle.StartTimer(timerEntity, expectedTimerDurationSeconds);
	
#pragma region Test started timer
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Calling %s on %s and checking that a new %s was created."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle::StartTimer),
			ARGUS_NAMEOF(TimerHandle),
			ARGUS_NAMEOF(Timer)
		),
		(timerComponent->m_timers[0].m_timeRemainingSeconds == expectedTimerDurationSeconds) &&
		(timerComponent->m_timers[0].m_timerState == TimerState::Ticking)
	);
#pragma endregion

#pragma region Test Starting a TimerHandle that has already been assigned
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Attempting to start a timer on a %s that is already assigned."),
			ARGUS_NAMEOF(TimerHandle)
		)
	);
#pragma endregion

	timerHandle.StartTimer(timerEntity, expectedTimerDurationSeconds);
	timerHandle.CancelTimer(timerEntity);
	timerHandle.StartTimer(timerEntity, expectedTimerDurationSeconds);

#pragma region Test started timer
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Calling %s on %s and checking that an existing %s was reused after getting reset."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle::StartTimer),
			ARGUS_NAMEOF(TimerHandle),
			ARGUS_NAMEOF(Timer)
		),
		(timerComponent->m_timers[0].m_timeRemainingSeconds == expectedTimerDurationSeconds) &&
		(timerComponent->m_timers[0].m_timerState == TimerState::Ticking) &&
		(timerComponent->m_timers.size() == 1)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TimerHandleFinishTimerTest, "Argus.ECS.TimerHandle.FinishTimer", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TimerHandleFinishTimerTest::RunTest(const FString& Parameters)
{
	const float expectedTimerDurationSeconds = 10.0f;

	ArgusTesting::StartArgusTest();

	ArgusEntity timerEntity = ArgusEntity::CreateEntity();
	TimerComponent* timerComponent = timerEntity.AddComponent<TimerComponent>();
	if (!timerComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	TimerHandle timerHandle;

#pragma region Test retrieving a Timer pointer from a TimerHandle
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Could not retrieve a %s from %s associated with passed in %s (%d)."),
			ARGUS_NAMEOF(Timer*),
			ARGUS_NAMEOF(TimerComponent),
			ARGUS_NAMEOF(ArgusEntity),
			timerEntity.GetId()
		)
	);
#pragma endregion

	timerHandle.FinishTimerHandling(timerEntity);
	timerHandle.StartTimer(timerEntity, expectedTimerDurationSeconds);
	timerHandle.FinishTimerHandling(timerEntity);

#pragma region Test finishing that is still in progress
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test finishing a %s that is still in progress."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(Timer)
		),
		(timerComponent->m_timers[0].m_timeRemainingSeconds == expectedTimerDurationSeconds) &&
		(timerComponent->m_timers[0].m_timerState == TimerState::Ticking)
	);
#pragma endregion

	timerComponent->m_timers[0].m_timerState = TimerState::Completed;
	timerHandle.FinishTimerHandling(timerEntity);

#pragma region Test finishing a completed timer.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test finishing a %s that has been completed."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(Timer)
		),
		(timerComponent->m_timers[0].m_timeRemainingSeconds == 0.0f) &&
		(timerComponent->m_timers[0].m_timerState == TimerState::NotSet)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TimerHandleCancelTimerTest, "Argus.ECS.TimerHandle.CancelTimer", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TimerHandleCancelTimerTest::RunTest(const FString& Parameters)
{
	const float expectedTimerDurationSeconds = 10.0f;

	ArgusTesting::StartArgusTest();
	ArgusEntity timerEntity = ArgusEntity::CreateEntity();
	TimerComponent* timerComponent = timerEntity.AddComponent<TimerComponent>();
	if (!timerComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

	TimerHandle timerHandle;

#pragma region Test retrieving a Timer pointer from a TimerHandle
	AddExpectedErrorPlain
	(
		FString::Printf
		(
			TEXT("Could not retrieve a %s from %s associated with passed in %s (%d)."),
			ARGUS_NAMEOF(Timer*),
			ARGUS_NAMEOF(TimerComponent),
			ARGUS_NAMEOF(ArgusEntity),
			timerEntity.GetId()
		)
	);
#pragma endregion

	timerHandle.CancelTimer(timerEntity);
	timerHandle.StartTimer(timerEntity, expectedTimerDurationSeconds);
	timerHandle.CancelTimer(timerEntity);

#pragma region Test finishing a completed timer.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test finishing a %s that has been completed."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(Timer)
		),
		(timerComponent->m_timers[0].m_timeRemainingSeconds == 0.0f) &&
		(timerComponent->m_timers[0].m_timerState == TimerState::NotSet)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TimerHandleGetTimerIndexTest, "Argus.ECS.TimerHandle.GetTimerIndex", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TimerHandleGetTimerIndexTest::RunTest(const FString& Parameters)
{
	const float expectedTimerDurationSeconds = 10.0f;
	const uint8 initialExpectedTimerIndex = UINT8_MAX;
	const uint8 expectedTimerIndex0 = 0u;
	const uint8 expectedTimerIndex1 = 1u;

	ArgusTesting::StartArgusTest();

	TimerHandle timerHandle0;
	TimerHandle timerHandle1;
	ArgusEntity timerEntity = ArgusEntity::CreateEntity();
	TimerComponent* timerComponent = timerEntity.AddComponent<TimerComponent>();
	if (!timerComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

#pragma region Test getting the timer index for an unassigned timer handle.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test calling %s on an unassigned %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle::GetTimerIndex),
			ARGUS_NAMEOF(TimerHandle)
		),
		timerHandle0.GetTimerIndex(),
		initialExpectedTimerIndex
	);
#pragma endregion

	timerHandle0.StartTimer(timerEntity, expectedTimerDurationSeconds);
	timerHandle1.StartTimer(timerEntity, expectedTimerDurationSeconds);

#pragma region Test getting the timer index for the first timer handle.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test calling %s on the first assigned %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle::GetTimerIndex),
			ARGUS_NAMEOF(TimerHandle)
		),
		timerHandle0.GetTimerIndex(),
		expectedTimerIndex0
	);
#pragma endregion

#pragma region Test getting the timer index for the second timer handle.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Test calling %s on the second assigned %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle::GetTimerIndex),
			ARGUS_NAMEOF(TimerHandle)
		),
		timerHandle1.GetTimerIndex(),
		expectedTimerIndex1
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(TimerHandleIsTimerCompleteTest, "Argus.ECS.TimerHandle.IsTimerComplete", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool TimerHandleIsTimerCompleteTest::RunTest(const FString& Parameters)
{
	const float expectedTimerDurationSeconds = 10.0f;
	ArgusTesting::StartArgusTest();

	TimerHandle timerHandle;
	ArgusEntity timerEntity = ArgusEntity::CreateEntity();
	TimerComponent* timerComponent = timerEntity.AddComponent<TimerComponent>();
	if (!timerComponent)
	{
		ArgusTesting::EndArgusTest();
		return false;
	}

#pragma region Test checking if an unassigned timer handle is complete
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Test checking if an unassigned %s is complete."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle)
		),
		timerHandle.IsTimerComplete(timerEntity)
	);
#pragma endregion

	timerHandle.StartTimer(timerEntity, expectedTimerDurationSeconds);

#pragma region Test checking if an assigned timer handle is complete
	TestFalse
	(
		FString::Printf
		(
			TEXT("[%s] Test checking if an assigned %s is complete."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle)
		),
		timerHandle.IsTimerComplete(timerEntity)
	);
#pragma endregion

	timerComponent->m_timers[0].m_timeRemainingSeconds = 0.0f;
	timerComponent->m_timers[0].m_timerState = TimerState::Completed;

#pragma region Test checking if an assigned timer handle is complete
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Test checking if an assigned %s is complete."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TimerHandle)
		),
		timerHandle.IsTimerComplete(timerEntity)
	);
#pragma endregion

	ArgusTesting::EndArgusTest();
	return true;
}

#endif //WITH_AUTOMATION_TESTS
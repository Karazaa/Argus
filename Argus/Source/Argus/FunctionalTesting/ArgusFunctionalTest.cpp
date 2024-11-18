// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusFunctionalTest.h"
#include "ArgusEntity.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AArgusFunctionalTest::AArgusFunctionalTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ArgusEntity::FlushAllEntities();
}

void AArgusFunctionalTest::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true);
}

void AArgusFunctionalTest::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);
	m_currentDeltaSeconds = deltaSeconds;

	if (!bIsRunning && GetWorld()->HasBegunPlay())
	{
		StartArgusFunctionalTest();
		return;
	}

	if (DidArgusFunctionalTestFail())
	{
		ConcludeFailedArgusFunctionalTest();
	}

	if (DidArgusFunctionalTestSucceed())
	{
		ConcludeSuccessfulArgusFunctionalTest();
	}
}

void AArgusFunctionalTest::OnTimeout()
{
	Super::OnTimeout();
	ExitArgusFunctionalTest();
}

bool AArgusFunctionalTest::DidArgusFunctionalTestSucceed()
{
	if (DidCurrentTestStepSucceed())
	{
		if (m_minimumSecondsForCurrentTestStep > 0.0f && !m_delayBetweenStepsTimer.IsSet())
		{
			m_delayBetweenStepsTimer = m_minimumSecondsForCurrentTestStep;
			return false;
		}

		if (m_delayBetweenStepsTimer.IsSet() && m_delayBetweenStepsTimer.GetValue() > 0.0f)
		{
			m_delayBetweenStepsTimer = m_delayBetweenStepsTimer.GetValue() - m_currentDeltaSeconds;
			return false;
		}

		if (m_testStepIndex >= GetTotalNumTestSteps() - 1)
		{
			return true;
		}

		FinishTestStep();
	}

	return false;
}

void AArgusFunctionalTest::StartArgusFunctionalTest()
{
	RunTest();
	StartNextTestStep();
}

void AArgusFunctionalTest::ConcludeSuccessfulArgusFunctionalTest()
{
	FinishTest(EFunctionalTestResult::Succeeded, *m_testSucceededMessage);
	ExitArgusFunctionalTest();
}

void AArgusFunctionalTest::ConcludeFailedArgusFunctionalTest()
{
	FinishTest(EFunctionalTestResult::Failed, *m_testFailedMessage);
	ExitArgusFunctionalTest();
}

void AArgusFunctionalTest::ExitArgusFunctionalTest()
{
	ArgusEntity::FlushAllEntities();
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	if (playerController)
	{
		playerController->ConsoleCommand("quit");
	}
}

void AArgusFunctionalTest::SetMinimumSecondsPerTestStep(float secondsPerTestStep)
{
	m_minimumSecondsForCurrentTestStep = secondsPerTestStep;
}

void AArgusFunctionalTest::FinishTestStep()
{
	m_minimumSecondsForCurrentTestStep = m_defaultMinimumSecondsPerTestStep;
	m_testStepIndex++;
	FinishStep();
	m_delayBetweenStepsTimer.Reset();
	StartNextTestStep();
}
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

	if (!bIsRunning && GetWorld()->HasBegunPlay())
	{
		StartArgusFunctionalTest();
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
		if (m_testStepIndex >= GetTotalNumTestSteps() - 1)
		{
			return true;
		}

		m_testStepIndex++;
		StartNextTestStep();
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
	FinishTest(EFunctionalTestResult::Succeeded, *m_testFailedMessage);
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
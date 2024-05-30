// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Argus_FunctionalTest_Navigation.h"

void AArgus_FunctionalTest_Navigation::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(true);
}

void AArgus_FunctionalTest_Navigation::Tick(float deltaSeconds)
{
	if (!bIsRunning && GetWorld()->HasBegunPlay())
	{
		StartNavigationTest();
	}
}

void AArgus_FunctionalTest_Navigation::StartNavigationTest()
{
	RunTest();

	if (!m_argusActor.IsValid())
	{
		FinishTest(EFunctionalTestResult::Failed, TEXT("Failed test due to invalid setup. m_argusActor must be a valid reference!"));
		return;
	}

	FinishTest(EFunctionalTestResult::Succeeded, TEXT("Test initialized successfully"));
}
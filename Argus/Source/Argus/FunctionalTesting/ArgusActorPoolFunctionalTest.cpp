// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorPoolFunctionalTest.h"

AArgusActorPoolFunctionalTest::AArgusActorPoolFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool AArgusActorPoolFunctionalTest::DidArgusFunctionalTestFail()
{
	return false;
}

bool AArgusActorPoolFunctionalTest::DidCurrentTestStepSucceed()
{
	if (DidArgusFunctionalTestFail())
	{
		return false;
	}

	switch (m_testStepIndex)
	{
		case 0:
			return DidInstantiateActorTestStepSucceed();
		default:
			break;
	}
	return false;
}

void AArgusActorPoolFunctionalTest::StartNextTestStep()
{
	if (DidArgusFunctionalTestFail())
	{
		return;
	}

	switch (m_testStepIndex)
	{
		case 0:
			StartInstantiateActorTestStep();
			break;
		default:
			break;
	}
}

void AArgusActorPoolFunctionalTest::StartInstantiateActorTestStep()
{
	StartStep(TEXT("Instantiate first actor object from pool."));
}

bool AArgusActorPoolFunctionalTest::DidInstantiateActorTestStepSucceed()
{
	return true;
}
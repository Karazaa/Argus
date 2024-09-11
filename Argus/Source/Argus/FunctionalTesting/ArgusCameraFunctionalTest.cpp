// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraFunctionalTest.h"
#include "ArgusMacros.h"

AArgusCameraFunctionalTest::AArgusCameraFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool AArgusCameraFunctionalTest::DidArgusFunctionalTestFail()
{
	return false;
}

void AArgusCameraFunctionalTest::StartNextTestStep()
{
	switch (m_testStepIndex)
	{
	case 0:
		break;
	default:
		break;
	}
}

bool AArgusCameraFunctionalTest::DidCurrentTestStepSucceed()
{
	switch (m_testStepIndex)
	{
	case 0:
		return true;
	default:
		break;
	}
	return false;
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "StaticDataFunctionalTest.h"

AStaticDataFunctionalTest::AStaticDataFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool AStaticDataFunctionalTest::DidArgusFunctionalTestFail()
{
	return false;
}

bool AStaticDataFunctionalTest::DidCurrentTestStepSucceed()
{
	return true;
}

void AStaticDataFunctionalTest::StartNextTestStep()
{

}
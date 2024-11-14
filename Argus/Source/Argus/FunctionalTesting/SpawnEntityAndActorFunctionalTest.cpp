// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpawnEntityAndActorFunctionalTest.h"
#include "ArgusEntity.h"
#include "ArgusGameModeBase.h"

ASpawnEntityAndActorFunctionalTest::ASpawnEntityAndActorFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool ASpawnEntityAndActorFunctionalTest::DidArgusFunctionalTestFail()
{
	const UArgusActorRecord* expectedArgusActorRecord = m_argusActorRecord.LoadSynchronous();
	if (!expectedArgusActorRecord)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActorRecord)
		);
		return true;
	}

	return false;
}

bool ASpawnEntityAndActorFunctionalTest::DidCurrentTestStepSucceed()
{
	if (DidArgusFunctionalTestFail())
	{
		return false;
	}

	switch (m_testStepIndex)
	{
		case 0:
			return true;
		default:
			return false;
	}
}

void ASpawnEntityAndActorFunctionalTest::StartNextTestStep()
{
	if (DidArgusFunctionalTestFail())
	{
		return;
	}

	switch (m_testStepIndex)
	{
		case 0:
			break;
		default:
			break;
	}
}
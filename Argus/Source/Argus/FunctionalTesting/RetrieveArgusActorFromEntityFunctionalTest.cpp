// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RetrieveArgusActorFromEntityFunctionalTest.h"
#include "ArgusActor.h"
#include "ArgusGameInstance.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Engine/World.h"

ARetrieveArgusActorFromEntityFunctionalTest::ARetrieveArgusActorFromEntityFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool ARetrieveArgusActorFromEntityFunctionalTest::DidArgusFunctionalTestFail()
{
	if (!m_argusActorToFind.IsValid())
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActorToFind)
		);
		return true;
	}

	return false;
}

void ARetrieveArgusActorFromEntityFunctionalTest::StartNextTestStep()
{
	switch (m_testStepIndex)
	{
	case 0:
		StartRetrieveArgusActorFromEntityTestStep();
		break;
	default:
		break;
	}
}

bool ARetrieveArgusActorFromEntityFunctionalTest::DidCurrentTestStepSucceed()
{
	switch (m_testStepIndex)
	{
	case 0:
		return DidRetrieveArgusActorFromEntityTestStepSucceed();
	default:
		break;
	}
	return false;
}

bool ARetrieveArgusActorFromEntityFunctionalTest::DidRetrieveArgusActorFromEntityTestStepSucceed()
{
	if (DidArgusFunctionalTestFail())
	{
		return false;
	}

	const UWorld* world = GetWorld();
	if (!world)
	{
		return false;
	}

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		return false;
	}

	AArgusActor* retrievedActor = gameInstance->GetArgusActorFromArgusEntity(m_argusActorToFind->GetEntity());

	if (retrievedActor)
	{
		m_testSucceededMessage = FString::Printf
		(
			TEXT("[%s] Successfully retrieving %s from an %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActorToFind),
			ARGUS_NAMEOF(ArgusEntity)
		);
		return true;
	}

	return false;
}

void ARetrieveArgusActorFromEntityFunctionalTest::StartRetrieveArgusActorFromEntityTestStep()
{
	StartStep(TEXT("Test retrieve ArgusActor from ArgusEntity."));
}
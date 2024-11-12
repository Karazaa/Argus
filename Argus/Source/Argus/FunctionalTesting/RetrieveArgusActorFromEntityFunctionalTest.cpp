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

void ARetrieveArgusActorFromEntityFunctionalTest::StartNextTestStep()
{
	switch (m_testStepIndex)
	{
		case 0u:
			StartRetrieveArgusActorFromEntityTestStep();
			break;
		case 1u:
			StartRemoveArgusActorFromEntityMapTestStep();
			break;
		default:
			break;
	}
}

bool ARetrieveArgusActorFromEntityFunctionalTest::DidCurrentTestStepSucceed()
{
	switch (m_testStepIndex)
	{
		case 0u:
			return DidRetrieveArgusActorFromEntityTestStepSucceed();
		case 1u:
			return DidRemoveArgusActorFromEntityMapTestStepSucceed();
		default:
			break;
	}
	return false;
}

void ARetrieveArgusActorFromEntityFunctionalTest::StartRetrieveArgusActorFromEntityTestStep()
{
	StartStep(TEXT("Test retrieve ArgusActor from ArgusEntity."));
}

void ARetrieveArgusActorFromEntityFunctionalTest::StartRemoveArgusActorFromEntityMapTestStep()
{
	m_argusActorToFind->Destroy();
	StartStep(TEXT("Test remove ArgusActor from ArgusEntity map."));
}

bool ARetrieveArgusActorFromEntityFunctionalTest::DidRetrieveArgusActorFromEntityTestStepSucceed()
{
	if (!m_argusActorToFind.IsValid())
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActorToFind)
		);
		return false;
	}

	const UWorld* world = GetWorld();
	if (!world)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(world)
		);
		return false;
	}

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(gameInstance)
		);
		return false;
	}

	m_cachedArgusEntity = m_argusActorToFind->GetEntity();
	AArgusActor* retrievedActor = gameInstance->GetArgusActorFromArgusEntity(m_cachedArgusEntity);

	if (retrievedActor && retrievedActor == m_argusActorToFind.Get())
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

bool ARetrieveArgusActorFromEntityFunctionalTest::DidRemoveArgusActorFromEntityMapTestStepSucceed()
{
	const UWorld* world = GetWorld();
	if (!world)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(world)
		);
		return false;
	}

	const UArgusGameInstance* gameInstance = world->GetGameInstance<UArgusGameInstance>();
	if (!gameInstance)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(gameInstance)
		);
		return false;
	}

	if (!m_cachedArgusEntity)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_cachedArgusEntity)
		);
		return false;
	}

	AArgusActor* retrievedActor = gameInstance->GetArgusActorFromArgusEntity(m_cachedArgusEntity);

	if (!retrievedActor)
	{
		m_testSucceededMessage = FString::Printf
		(
			TEXT("[%s] Successfully destroyed %s and could not retrieve a reference from %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActorToFind),
			ARGUS_NAMEOF(UArgusGameInstance::GetArgusActorFromArgusEntity)
		);
		return true;
	}

	return false;
}
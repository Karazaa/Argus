// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorPoolFunctionalTest.h"

AArgusActorPoolFunctionalTest::AArgusActorPoolFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool AArgusActorPoolFunctionalTest::DidArgusFunctionalTestFail()
{
	if (!m_class0 || !m_class1)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. Must set %s references."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TSoftClassPtr<AArgusActor>)
		);
		return true;
	}

	if (!m_entityTemplate0 || !m_entityTemplate1)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. Must set %s references."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(TObjectPtr<UArgusEntityTemplate>)
		);
		return true;
	}

	UClass* class0 = m_class0.LoadSynchronous();
	UClass* class1 = m_class1.LoadSynchronous();
	if (!class0 || !class1)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. Could not retrieve %s pointers from %s references."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(UClass),
			ARGUS_NAMEOF(TSoftClassPtr<AArgusActor>)
		);
		return true;
	}

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
		case 0u:
			return DidInstantiateActorTestStepSucceed();
		case 1u:
			return DidReleaseActorTestStepSucceed();
		case 2u:
			return DidReleaseActorTestStepSucceed();
		case 3u:
			return DidReleaseSecondActorTestStepSucceed();
		case 4u:
			return DidTakeInstantiatedActorTestStepSucceed();
		case 5u:
			return DidMakeEntityForActorTestStepSucceed();
		case 6u:
			return DidTakeSecondInstantiatedActorTestStepSucceed();
		case 7u:
			return DidMakeEntityForSecondActorTestStepSucceed();
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
		case 0u:
			StartInstantiateActorTestStep();
			break;
		case 1u:
			StartReleaseActorTestStep();
			break;
		case 2u:
			StartInstantiateSecondActorTestStep();
			break;
		case 3u:
			StartReleaseSecondActorTestStep();
			break;
		case 4u:
			StartTakeInstantiatedActorTestStep();
			break;
		case 5u:
			StartMakeEntityForActorTestStep();
			break;
		case 6u:
			StartTakeSecondInstantiatedActorTestStep();
			break;
		case 7u:
			StartMakeEntityForSecondActorTestStep();
			break;
		default:
			break;
	}
}

void AArgusActorPoolFunctionalTest::StartInstantiateActorTestStep()
{
	StartStep(TEXT("Instantiate first actor object from pool."));

	m_actor0 = m_argusActorPool.Take(GetWorld(), m_class0.LoadSynchronous());
}

void AArgusActorPoolFunctionalTest::StartReleaseActorTestStep()
{
	StartStep(TEXT("Release first instantiated actor back to pool."));

	m_argusActorPool.Release(m_actor0);
}

void AArgusActorPoolFunctionalTest::StartInstantiateSecondActorTestStep()
{
	StartStep(TEXT("Instantiate second actor object from pool."));

	m_actor1 = m_argusActorPool.Take(GetWorld(), m_class1.LoadSynchronous());
}

void AArgusActorPoolFunctionalTest::StartReleaseSecondActorTestStep()
{
	StartStep(TEXT("Release second instantiated actor back to pool."));

	m_argusActorPool.Release(m_actor1);
}

void AArgusActorPoolFunctionalTest::StartTakeInstantiatedActorTestStep()
{
	StartStep(TEXT("Take already instantiated first actor object from pool."));

	m_actor0 = m_argusActorPool.Take(GetWorld(), m_class0.LoadSynchronous());
}

void AArgusActorPoolFunctionalTest::StartMakeEntityForActorTestStep()
{
	StartStep(TEXT("Make an entity for the actor retrieved from the pool and set it."));

	if (m_actor0 && m_entityTemplate0)
	{
		m_actor0->SetEntity(m_entityTemplate0->MakeEntity());
	}
}

void AArgusActorPoolFunctionalTest::StartTakeSecondInstantiatedActorTestStep()
{
	StartStep(TEXT("Take already instantiated second actor object from pool."));

	m_actor1 = m_argusActorPool.Take(GetWorld(), m_class1.LoadSynchronous());
}

void AArgusActorPoolFunctionalTest::StartMakeEntityForSecondActorTestStep()
{
	StartStep(TEXT("Make an entity for the second actor retrieved from the pool and set it."));

	if (m_actor1 && m_entityTemplate1)
	{
		m_actor1->SetEntity(m_entityTemplate1->MakeEntity());
	}
}

bool AArgusActorPoolFunctionalTest::DidInstantiateActorTestStepSucceed()
{
	UClass* class0 = m_class0.LoadSynchronous();
	if (m_actor0 && class0 && !m_actor0->GetEntity() && !m_actor0->IsHidden() && m_actor0->IsA(class0) && m_argusActorPool.GetNumAvailableObjects() == 0u)
	{
		return true;
	}

	return false;
}

bool AArgusActorPoolFunctionalTest::DidReleaseActorTestStepSucceed()
{
	if (!m_actor0 && m_argusActorPool.GetNumAvailableObjects() == 1u)
	{
		return true;
	}

	return false;
}

bool AArgusActorPoolFunctionalTest::DidInstantiateSecondActorTestStepSucceed()
{
	UClass* class1 = m_class1.LoadSynchronous();
	if (m_actor1 && class1 && !m_actor1->GetEntity() && !m_actor1->IsHidden() && m_actor1->IsA(class1) && m_argusActorPool.GetNumAvailableObjects() == 1u)
	{
		return true;
	}

	return false;
}

bool AArgusActorPoolFunctionalTest::DidReleaseSecondActorTestStepSucceed()
{
	if (!m_actor1 && m_argusActorPool.GetNumAvailableObjects() == 2u)
	{
		return true;
	}

	return false;
}

bool AArgusActorPoolFunctionalTest::DidTakeInstantiatedActorTestStepSucceed()
{
	UClass* class0 = m_class0.LoadSynchronous();
	if (m_actor0 && class0 && !m_actor0->GetEntity() && !m_actor0->IsHidden() && m_actor0->IsA(class0) && m_argusActorPool.GetNumAvailableObjects() == 1u)
	{
		return true;
	}

	return false;
}

bool AArgusActorPoolFunctionalTest::DidMakeEntityForActorTestStepSucceed()
{
	if (m_actor0 && m_actor0->GetEntity())
	{
		return true;
	}

	return false;
}

bool AArgusActorPoolFunctionalTest::DidTakeSecondInstantiatedActorTestStepSucceed()
{
	UClass* class1 = m_class1.LoadSynchronous();
	if (m_actor1 && class1 && !m_actor1->GetEntity() && !m_actor1->IsHidden() && m_actor1->IsA(class1) && m_argusActorPool.GetNumAvailableObjects() == 0u)
	{
		return true;
	}

	return false;
}

bool AArgusActorPoolFunctionalTest::DidMakeEntityForSecondActorTestStepSucceed()
{
	if (m_actor1 && m_actor1->GetEntity())
	{
		return true;
	}

	return false;
}

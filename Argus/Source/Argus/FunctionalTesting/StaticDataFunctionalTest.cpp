// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "StaticDataFunctionalTest.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"

AStaticDataFunctionalTest::AStaticDataFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool AStaticDataFunctionalTest::DidArgusFunctionalTestFail()
{
	if (!m_factionRecord.IsValid())
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_factionRecord)
		);
		return true;
	}

	return false;
}

void AStaticDataFunctionalTest::StartNextTestStep()
{
	switch (m_testStepIndex)
	{
		case 0:
			StartFactionTestStep();
			break;
		default:
			break;
	}
}

bool AStaticDataFunctionalTest::DidCurrentTestStepSucceed()
{
	switch (m_testStepIndex)
	{
		case 0:
			return DidFactionTestStepSucceed();
		default:
			break;
	}
	return false;
}

bool AStaticDataFunctionalTest::DidFactionTestStepSucceed()
{
	if (DidArgusFunctionalTestFail())
	{
		return false;
	}

	const UFactionRecord* expectedFactionRecord = m_factionRecord.LoadSynchronous();
	const UFactionRecord* retrievedFactionRecord = ArgusStaticData::GetRecord<UFactionRecord>(expectedFactionRecord->m_id);

	if (!retrievedFactionRecord)
	{
		return false;
	}

	const bool matchId = expectedFactionRecord->m_id == retrievedFactionRecord->m_id;
	const bool matchFactionName = expectedFactionRecord->m_factionName == retrievedFactionRecord->m_factionName;

	return matchId && matchFactionName;
}

void AStaticDataFunctionalTest::StartFactionTestStep()
{
	StartStep(TEXT("Test retrieve faction data."));
}
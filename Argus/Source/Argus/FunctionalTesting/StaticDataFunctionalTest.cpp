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
	const UFactionRecord* expectedFactionRecord = m_factionRecord.LoadSynchronous();
	if (!expectedFactionRecord)
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
		case 0u:
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
		case 0u:
			return DidFactionTestStepSucceed();
		default:
			break;
	}
	return false;
}

void AStaticDataFunctionalTest::StartFactionTestStep()
{
	StartStep(TEXT("Test retrieve faction data."));
}

bool AStaticDataFunctionalTest::DidFactionTestStepSucceed()
{
	if (DidArgusFunctionalTestFail())
	{
		return false;
	}

	const UFactionRecord* expectedFactionRecord = m_factionRecord.LoadSynchronous();
	if (!expectedFactionRecord)
	{
		return false;
	}

	const uint32 expectedFactionId = expectedFactionRecord->m_id;
	const UFactionRecord* retrievedFactionRecord = ArgusStaticData::GetRecord<UFactionRecord>(expectedFactionId);
	if (!retrievedFactionRecord)
	{
		return false;
	}
	const uint32 retrievedFactionId = retrievedFactionRecord->m_id;

	const bool matchesName = expectedFactionRecord->m_factionName == retrievedFactionRecord->m_factionName;
	const bool matchesId = expectedFactionId == retrievedFactionId;

	if (matchesName && matchesId)
	{
		m_testSucceededMessage = FString::Printf
		(
			TEXT("[%s] Successfully loaded %s and validated data."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_factionRecord)
		);
		return true;
	}
	else
	{
		return false;
	}
}
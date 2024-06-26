// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusFunctionalTest.h"
#include "RecordDefinitions/FactionRecord.h"
#include "StaticDataFunctionalTest.generated.h"

UCLASS()
class ARGUS_API AStaticDataFunctionalTest : public AArgusFunctionalTest
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.StaticData";
	static constexpr uint8       k_totalNumSteps = 1;

public:
	AStaticDataFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UFactionRecord> m_factionRecord;

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override { return k_totalNumSteps; }

private:
	bool DidFactionTestStepSucceed();
	void StartFactionTestStep();
};
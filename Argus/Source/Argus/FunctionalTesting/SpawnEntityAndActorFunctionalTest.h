// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusFunctionalTest.h"
#include "RecordDefinitions/ArgusActorRecord.h"
#include "SpawnEntityAndActorFunctionalTest.generated.h"

UCLASS()
class ARGUS_API ASpawnEntityAndActorFunctionalTest : public AArgusFunctionalTest
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.SpawnEntityAndActor";
	static constexpr uint8       k_totalNumSteps = 1u;

public:
	ASpawnEntityAndActorFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UArgusActorRecord> m_argusActorRecord;

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override { return k_totalNumSteps; }
};
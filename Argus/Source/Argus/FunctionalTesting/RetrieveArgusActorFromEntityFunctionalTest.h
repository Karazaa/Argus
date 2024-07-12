// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusFunctionalTest.h"
#include "RetrieveArgusActorFromEntityFunctionalTest.generated.h"

class AArgusActor;

UCLASS()
class ARGUS_API ARetrieveArgusActorFromEntityFunctionalTest : public AArgusFunctionalTest
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.RetrieveArgusActorFromEntity";
	static constexpr uint8       k_totalNumSteps = 1;

public:
	ARetrieveArgusActorFromEntityFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusActor> m_argusActorToFind = nullptr;

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override { return k_totalNumSteps; }

private:
	bool DidRetrieveArgusActorFromEntityTestStepSucceed();
	void StartRetrieveArgusActorFromEntityTestStep();
};
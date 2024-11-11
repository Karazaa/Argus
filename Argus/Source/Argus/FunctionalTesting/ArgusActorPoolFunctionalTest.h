// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "ArgusFunctionalTest.h"
#include "ArgusActorPoolFunctionalTest.generated.h"

UCLASS()
class ARGUS_API AArgusActorPoolFunctionalTest : public AArgusFunctionalTest 
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.ArgusActorPool";
	static constexpr uint8       k_totalNumSteps = 1;

public:
	AArgusActorPoolFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AArgusActor> m_class0 = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UArgusEntityTemplate> m_entityTemplate0 = nullptr;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AArgusActor> m_class1 = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UArgusEntityTemplate> m_entityTemplate1 = nullptr;

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override { return k_totalNumSteps; }

private:
	void StartInstantiateActorTestStep();

	bool DidInstantiateActorTestStepSucceed();
};
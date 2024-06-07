// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "ArgusFunctionalTest.h"
#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "Argus_FunctionalTest_Navigation.generated.h"

UCLASS()
class ARGUS_API AArgus_FunctionalTest_Navigation : public AArgusFunctionalTest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusActor> m_argusActor = nullptr;

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AActor> m_goalActor = nullptr;

	UPROPERTY(EditAnywhere)
	float m_successDistance = 0.0f;

protected:
	virtual bool DidArgusFunctionalTestSucceed() override;
	virtual bool DidArgusFunctionalTestFail() override;
	virtual void StartArgusFunctionalTest() override;

private:
	static constexpr uint8 k_totalNumSteps = 2;
	uint8 m_testStepIndex = 0;

	void StartNextTestStep();
	void StartNavigationToLocationStep();
	void StartNavigationToEntityStep();

	bool DidCurrentTestStepSucceed();
	bool DidNavigationToLocationStepSucceed();
	bool DidNavigationToEntityStepSucceed();
};

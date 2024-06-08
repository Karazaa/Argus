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

private:
	static constexpr uint8 k_totalNumSteps = 2;

public:
	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusActor> m_argusActor = nullptr;

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AActor> m_goalActor = nullptr;

	UPROPERTY(EditAnywhere)
	float m_successDistance = 0.0f;

protected:
	virtual bool  DidArgusFunctionalTestFail() override;
	virtual bool  DidCurrentTestStepSucceed() override;
	virtual void  StartNextTestStep() override;
	virtual uint8 GetTotalNumTestSteps() override { return k_totalNumSteps; }

private:
	void StartNavigationToLocationStep();
	void StartNavigationToEntityStep();
	bool DidNavigationToLocationStepSucceed();
	bool DidNavigationToEntityStepSucceed();
};

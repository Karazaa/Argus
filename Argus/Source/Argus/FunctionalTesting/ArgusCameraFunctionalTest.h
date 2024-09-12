// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusFunctionalTest.h"
#include "ArgusCameraFunctionalTest.generated.h"

class AArgusCameraActor;

UCLASS()
class ARGUS_API AArgusCameraFunctionalTest : public AArgusFunctionalTest
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.ArgusCamera";
	static constexpr uint8       k_totalNumSteps = 4;

public:
	AArgusCameraFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusCameraActor> m_argusCameraActor = nullptr;

	UPROPERTY(EditAnywhere)
	float m_targetPanLeftRightQuantity = 3000.0f;

	UPROPERTY(EditAnywhere)
	float m_targetPanUpDownQuantity = 3000.0f;

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override { return k_totalNumSteps; }

private:
	void StartPanLeftTestStep();
	void StartPanRightTestStep();
	void StartPanUpTestStep();
	void StartPanDownTestStep();
	bool DidPanLeftTestStepSucceed(const FVector& locationDifference);
	bool DidPanRightTestStepSucceed(const FVector& locationDifference);
	bool DidPanUpTestStepSucceed(const FVector& locationDifference);
	bool DidPanDownTestStepSucceed(const FVector& locationDifference);

	FVector m_testStartingLocation = FVector::ZeroVector;
	TOptional<float> m_stepDelayTimer;
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "FunctionalTest.h"
#include "ArgusFunctionalTest.generated.h"

UCLASS()
class ARGUS_API AArgusFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AArgusFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	float m_secondsBetweenSteps = 0.0f;

protected:
	FString m_testSucceededMessage = FString();
	FString m_testFailedMessage = FString();
	uint8 m_testStepIndex = 0;
	float m_currentDeltaSeconds = 0.1f;
	TOptional<float> m_delayBetweenStepsTimer;

	virtual bool IsReady_Implementation()	override	{ return true; }
	virtual void BeginPlay()				override;
	virtual void Tick(float deltaSeconds)	override;
	virtual void OnTimeout()				override;

	virtual bool    DidArgusFunctionalTestFail()		{ return false; }
	virtual bool    DidCurrentTestStepSucceed()			{ return false; }
	virtual uint8   GetTotalNumTestSteps()				{ return 0u; };
	virtual void    StartNextTestStep()					{};

	virtual bool DidArgusFunctionalTestSucceed();
	virtual void StartArgusFunctionalTest();
	virtual void ConcludeSuccessfulArgusFunctionalTest();
	virtual void ConcludeFailedArgusFunctionalTest();
	virtual void ExitArgusFunctionalTest();
};
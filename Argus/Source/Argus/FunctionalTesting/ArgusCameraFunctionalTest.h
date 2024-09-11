// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusFunctionalTest.h"
#include "ArgusCameraFunctionalTest.generated.h"

UCLASS()
class ARGUS_API AArgusCameraFunctionalTest : public AArgusFunctionalTest
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.ArgusCamera";
	static constexpr uint8       k_totalNumSteps = 1;

public:
	AArgusCameraFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override { return k_totalNumSteps; }

};
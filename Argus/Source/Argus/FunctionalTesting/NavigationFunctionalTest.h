// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "ArgusFunctionalTest.h"
#include "NavigationFunctionalTest.generated.h"

UCLASS()
class ARGUS_API ANavigationFunctionalTest : public AArgusFunctionalTest
{
	GENERATED_BODY()

private:
	static constexpr const char* k_testName = "Argus.FunctionalTest.Navigation";
	static constexpr uint8       k_totalNumSteps = 3;

public:
	ANavigationFunctionalTest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusActor> m_argusActor = nullptr;

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusActor> m_goalEntityActor = nullptr;

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AActor> m_goalLocationActor = nullptr;

	UPROPERTY(EditAnywhere)
	float m_successDistance = 0.0f;

protected:
	virtual bool    DidArgusFunctionalTestFail()	override;
	virtual bool    DidCurrentTestStepSucceed()		override;
	virtual void    StartNextTestStep()				override;
	virtual uint8   GetTotalNumTestSteps()			override	{ return k_totalNumSteps; }

private:
	void StartNavigationToLocationTestStep();
	void StartNavigationToEntityTestStep();
	void StartNavigationThroughWaypointsTestStep();
	bool DidNavigationToLocationTestStepSucceed();
	bool DidNavigationToEntityTestStepSucceed();
	bool DidNavigationThroughWaypointsTestStepSucceed();

	FVector m_waypoint0 = FVector::ZeroVector;
	FVector m_waypoint1 = FVector::ZeroVector;
	FVector m_waypoint2 = FVector::ZeroVector;
};

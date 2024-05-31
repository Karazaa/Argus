// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "Argus_FunctionalTest_Navigation.generated.h"

UCLASS()
class ARGUS_API AArgus_FunctionalTest_Navigation : public AFunctionalTest
{
	GENERATED_BODY()

public:
	AArgus_FunctionalTest_Navigation(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusActor> m_argusActor = nullptr;

	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AActor> m_goalActor = nullptr;

	UPROPERTY(EditAnywhere)
	float m_successDistance = 0.0f;

protected:
	virtual bool IsReady_Implementation() override { return true; }
	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;
	virtual void OnTimeout() override;

private:
	void StartNavigationTest();
	void CheckIfSucceeded();
	void ExitTest();
};

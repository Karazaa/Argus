// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "Argus_FunctionalTest_Navigation.generated.h"

/**
 * 
 */
UCLASS()
class ARGUS_API AArgus_FunctionalTest_Navigation : public AFunctionalTest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AArgusActor> m_argusActor = nullptr;

protected:
	virtual bool IsReady_Implementation() override { return true; }
	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;

private:
	void StartNavigationTest();
};

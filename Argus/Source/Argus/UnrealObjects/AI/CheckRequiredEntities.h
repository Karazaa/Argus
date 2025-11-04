// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include <ArgusAIController.h>

#include "CheckRequiredEntities.generated.h"


/**
 * 
 */
UCLASS()
class ARGUS_API UCheckRequiredEntities : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BLueprintReadWrite, EditAnywhere)
	AArgusAIController* m_aiController;

	EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) override;

	void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	
};

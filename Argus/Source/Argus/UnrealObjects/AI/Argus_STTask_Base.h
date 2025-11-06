// Copyright Wayne Potts

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "ArgusAIController.h"
#include "Argus_STTask_Base.generated.h"

/**
 * 
 */
UCLASS()
class ARGUS_API UArgus_STTask_Base : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BLueprintReadWrite, EditAnywhere)
	AArgusAIController* m_aiController;

	EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

	virtual void OnArgusStartTask() {};
};

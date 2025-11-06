// Copyright Wayne Potts


#include "Argus_STTask_Base.h"

EStateTreeRunStatus UArgus_STTask_Base::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    if (!bShouldStateChangeOnReselect && Transition.ChangeType == EStateTreeStateChangeType::Sustained) {
        // Unreal is having internal issues, so just exit with what it was going to do anyway
        return EStateTreeRunStatus::Running;
    }

    OnArgusStartTask();

    return Super::EnterState(Context, Transition);
}

void UArgus_STTask_Base::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    if (!bShouldStateChangeOnReselect && Transition.ChangeType == EStateTreeStateChangeType::Sustained) {
        // Unreal is having some internal issues, so just exit without doing anything
        return;

    }

    // Tear down any events etc

    Super::ExitState(Context, Transition);
}
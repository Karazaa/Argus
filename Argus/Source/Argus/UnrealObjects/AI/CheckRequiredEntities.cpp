// Copyright Karazaa. This is a part of an RTS project called Argus.



#include "CheckRequiredEntities.h"

EStateTreeRunStatus UCheckRequiredEntities::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    if (!bShouldStateChangeOnReselect && Transition.ChangeType == EStateTreeStateChangeType::Sustained) {
        // Unreal has fucked up if this happens, so just exit with what it was going to do anyway
        return EStateTreeRunStatus::Running;
    }
    
    FinishTask(!m_aiController->HasRequiredEntities());
    
    return Super::EnterState(Context, Transition);
}

void UCheckRequiredEntities::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
    if (!bShouldStateChangeOnReselect && Transition.ChangeType == EStateTreeStateChangeType::Sustained) {
        // Unreal has fucked up if this happens, so just exit without doing anything
        return;

    }

    // Tear down any events etc

    Super::ExitState(Context, Transition);
}

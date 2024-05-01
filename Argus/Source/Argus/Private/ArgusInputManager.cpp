// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusInputActionSet.h"
#include "ArgusUtil.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"

void ArgusInputManager::SetupInputComponent(TObjectPtr<UInputComponent>& inputComponent, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet)
{
	if (!inputComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null input component, %s, passed into %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(inputComponent), ARGUS_FUNCNAME);
		return;
	}

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(inputComponent.Get());
	if (!enhancedInputComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Failed to cast input component, %s, to a %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(inputComponent), ARGUS_NAMEOF(UEnhancedInputComponent));
		return;
	}

	UArgusInputActionSet* actionSet = argusInputActionSet.LoadSynchronous();
	if (!actionSet)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Failed to retrieve input action set, %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(argusInputActionSet));
		return;
	}

	// TODO JAMES: Bind actions.
	// enhancedInputComponent->BindAction()
}

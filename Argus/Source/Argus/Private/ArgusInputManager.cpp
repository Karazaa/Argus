// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusInputActionSet.h"
#include "ArgusUtil.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"

void UArgusInputManager::SetupInputComponent(TObjectPtr<UInputComponent>& inputComponent, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet)
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

	if (const UInputAction* selectAction = actionSet->m_selectAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(selectAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSelect);
	}

	if (const UInputAction* moveToAction = actionSet->m_moveToAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(moveToAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnMoveTo);
	}	
}

void UArgusInputManager::OnSelect(const FInputActionValue& value)
{
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] was called!"), ARGUS_FUNCNAME);
}

void UArgusInputManager::OnMoveTo(const FInputActionValue& value)
{
	UE_LOG(ArgusGameLog, Display, TEXT("[%s] was called!"), ARGUS_FUNCNAME);
}

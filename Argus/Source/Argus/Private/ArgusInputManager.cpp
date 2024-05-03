// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusPlayerController.h"
#include "ArgusUtil.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"

void UArgusInputManager::SetupInputComponent(TWeakObjectPtr<AArgusPlayerController> owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet)
{
	if (!owningPlayerController.IsValid())
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] %s is setting up an InputComponent without a valid owning %s"), ARGUS_FUNCNAME, ARGUS_NAMEOF(UArgusInputManager), ARGUS_NAMEOF(AArgusPlayerController));
		return;
	}
	m_owningPlayerController = owningPlayerController;

	if (!m_owningPlayerController->InputComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null input component, %s, assigned to %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController->InputComponent), ARGUS_NAMEOF(m_owningPlayerController));
		return;
	}

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(m_owningPlayerController->InputComponent.Get());
	if (!enhancedInputComponent)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Failed to cast input component, %s, to a %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController->InputComponent), ARGUS_NAMEOF(UEnhancedInputComponent));
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
	if (!m_owningPlayerController.IsValid())
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null %s, assigned in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController), ARGUS_NAMEOF(UArgusInputManager));
		return;
	}

	FHitResult hitResult;
	if (m_owningPlayerController->GetMouseProjectionLocation(hitResult))
	{
		UE_LOG(ArgusGameLog, Display, TEXT("[%s] was called! Mouse projection worldspace location is (%f, %f, %f)"), ARGUS_FUNCNAME, hitResult.Location.X, hitResult.Location.Y, hitResult.Location.Z);
	}
}

void UArgusInputManager::OnMoveTo(const FInputActionValue& value)
{
	if (!m_owningPlayerController.IsValid())
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null %s, assigned in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController), ARGUS_NAMEOF(UArgusInputManager));
		return;
	}

	FHitResult hitResult;
	if (m_owningPlayerController->GetMouseProjectionLocation(hitResult))
	{
		UE_LOG(ArgusGameLog, Display, TEXT("[%s] was called! Mouse projection worldspace location is (%f, %f, %f)"), ARGUS_FUNCNAME, hitResult.Location.X, hitResult.Location.Y, hitResult.Location.Z);
	}
}
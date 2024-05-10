// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusActor.h"
#include "ArgusPlayerController.h"
#include "ArgusUtil.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"

void UArgusInputManager::ProcessPlayerInput()
{
	// TODO JAMES: Something lol
}

void UArgusInputManager::SetupInputComponent(TWeakObjectPtr<AArgusPlayerController> owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet)
{
	if (!owningPlayerController.IsValid())
	{
		UE_LOG
		(
			ArgusGameLog, Error, TEXT("[%s] %s is setting up an InputComponent without a valid owning %s"), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(UArgusInputManager), 
			ARGUS_NAMEOF(AArgusPlayerController)
		);
		return;
	}
	m_owningPlayerController = owningPlayerController;

	if (!m_owningPlayerController->InputComponent)
	{
		UE_LOG
		(
			ArgusGameLog, Error, TEXT("[%s] Null input component, %s, assigned to %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(m_owningPlayerController->InputComponent), 
			ARGUS_NAMEOF(m_owningPlayerController)
		);
		return;
	}

	TWeakObjectPtr<UEnhancedInputComponent> enhancedInputComponent = Cast<UEnhancedInputComponent>(m_owningPlayerController->InputComponent.Get());	
	BindActions(argusInputActionSet, enhancedInputComponent);
}

void UArgusInputManager::OnSelect(const FInputActionValue& value)
{
	OnSelectInternal(false);
}

void UArgusInputManager::OnSelectAdditive(const FInputActionValue& value)
{
	OnSelectInternal(true);
}

void UArgusInputManager::OnMoveTo(const FInputActionValue& value)
{
	if (!ValidateOwningPlayerController())
	{
		return;
	}

	FHitResult hitResult;
	if (m_owningPlayerController->GetMouseProjectionLocation(hitResult))
	{
		UE_LOG
		(
			ArgusGameLog, Display, TEXT("[%s] was called! Mouse projection worldspace location is (%f, %f, %f)"), 
			ARGUS_FUNCNAME, 
			hitResult.Location.X, 
			hitResult.Location.Y, 
			hitResult.Location.Z
		);

		const int numSelectedEntities = m_selectedArgusActors.Num();
		if (numSelectedEntities)
		{
			UE_LOG(ArgusGameLog, Display, TEXT("[%s] was called while %d entities were selected."), ARGUS_FUNCNAME, numSelectedEntities);
		}
	}
}

void UArgusInputManager::BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, TWeakObjectPtr<UEnhancedInputComponent>& enhancedInputComponent)
{
	if (!enhancedInputComponent.IsValid())
	{
		UE_LOG
		(
			ArgusGameLog, Error, TEXT("[%s] Failed to cast input component, %s, to a %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(m_owningPlayerController->InputComponent), 
			ARGUS_NAMEOF(UEnhancedInputComponent)
		);
		return;
	}

	const UArgusInputActionSet* actionSet = argusInputActionSet.LoadSynchronous();
	if (!actionSet)
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Failed to retrieve input action set, %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(argusInputActionSet));
		return;
	}

	if (const UInputAction* selectAction = actionSet->m_selectAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(selectAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSelect);
	}

	if (const UInputAction* selectAdditiveAction = actionSet->m_selectAdditiveAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(selectAdditiveAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSelectAdditive);
	}

	if (const UInputAction* moveToAction = actionSet->m_moveToAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(moveToAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnMoveTo);
	}
}

void UArgusInputManager::OnSelectInternal(bool isAdditive)
{
	if (!ValidateOwningPlayerController())
	{
		return;
	}

	FHitResult hitResult;
	if (m_owningPlayerController->GetMouseProjectionLocation(hitResult))
	{
		UE_LOG
		(
			ArgusGameLog, Display, TEXT("[%s] was called! Mouse projection worldspace location is (%f, %f, %f). Is Additive? %s"), 
			ARGUS_FUNCNAME, 
			hitResult.Location.X, 
			hitResult.Location.Y, 
			hitResult.Location.Z, 
			isAdditive ? TEXT("Yes") : TEXT("No")
		);
	}

	if (AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor()))
	{
		if (!isAdditive)
		{
			m_selectedArgusActors.Empty();
		}
		
		ArgusEntity entity = argusActor->GetEntity();
		UE_LOG(ArgusGameLog, Display, TEXT("[%s] selected an %s with ID %d"), ARGUS_FUNCNAME, ARGUS_NAMEOF(AArgusActor), entity.GetId());
		m_selectedArgusActors.Emplace(argusActor);
	}
}

bool UArgusInputManager::ValidateOwningPlayerController()
{
	if (!m_owningPlayerController.IsValid())
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null %s, assigned in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController), ARGUS_NAMEOF(UArgusInputManager));
		return false;
	}

	return true;
}
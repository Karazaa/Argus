// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusActor.h"
#include "ArgusPlayerController.h"
#include "ArgusUtil.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"

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
	m_inputEventsThisFrame.Emplace(InputType::Select);
}

void UArgusInputManager::OnSelectAdditive(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputType::SelectAdditive);
}

void UArgusInputManager::OnMoveTo(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputType::MoveTo);
}

void UArgusInputManager::ProcessPlayerInput()
{
	const int inputsEventsThisFrameCount = m_inputEventsThisFrame.Num();
	for (int i = 0; i < inputsEventsThisFrameCount; ++i)
	{
		ProcessInputEvent(m_inputEventsThisFrame[i]);
	}
	m_inputEventsThisFrame.Empty();
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

bool UArgusInputManager::ValidateOwningPlayerController()
{
	if (!m_owningPlayerController.IsValid())
	{
		UE_LOG(ArgusGameLog, Error, TEXT("[%s] Null %s, assigned in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController), ARGUS_NAMEOF(UArgusInputManager));
		return false;
	}

	return true;
}

void UArgusInputManager::ProcessInputEvent(InputType inputType)
{
	switch (inputType)
	{
		case InputType::Select:
			ProcessSelectInputEvent(false);
			break;
		case InputType::SelectAdditive:
			ProcessSelectInputEvent(true);
			break;
		case InputType::MoveTo:
			ProcessMoveToInputEvent();
			break;
		default:
			break;
	}
}

void UArgusInputManager::ProcessSelectInputEvent(bool isAdditive)
{
	if (!ValidateOwningPlayerController())
	{
		return;
	}

	FHitResult hitResult;
	if (!m_owningPlayerController->GetMouseProjectionLocation(hitResult))
	{
		return;
	}

	if (AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor()))
	{
		if (isAdditive)
		{
			if (m_selectedArgusActors.Contains(argusActor))
			{
				argusActor->SetSelectionState(false);
				m_selectedArgusActors.Remove(argusActor);
			}
			else
			{
				argusActor->SetSelectionState(true);
				m_selectedArgusActors.Emplace(argusActor);
			}
		}
		else
		{
			bool alreadySelected = false;
			for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
			{
				if (selectedActor.GetEvenIfUnreachable() == argusActor)
				{
					alreadySelected = true;
				}
				else if (selectedActor.IsValid())
				{
					selectedActor->SetSelectionState(false);
				}
			}
			m_selectedArgusActors.Empty();

			if (!alreadySelected)
			{
				argusActor->SetSelectionState(true);
			}
			m_selectedArgusActors.Emplace(argusActor);
		}

		UE_LOG
		(
			ArgusGameLog, Display, TEXT("[%s] selected an %s with ID %d. Is additive? %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(AArgusActor),
			argusActor->GetEntity().GetId(),
			isAdditive ? TEXT("Yes") : TEXT("No")
		);
	}
}

void UArgusInputManager::ProcessMoveToInputEvent()
{
	if (!ValidateOwningPlayerController())
	{
		return;
	}

	FHitResult hitResult;
	if (!m_owningPlayerController->GetMouseProjectionLocation(hitResult))
	{
		return;
	}

	UE_LOG
	(
		ArgusGameLog, Display, TEXT("[%s] Move To input occurred. Mouse projection worldspace location is (%f, %f, %f)"),
		ARGUS_FUNCNAME,
		hitResult.Location.X,
		hitResult.Location.Y,
		hitResult.Location.Z
	);

	const int numSelectedEntities = m_selectedArgusActors.Num();
	if (numSelectedEntities)
	{
		UE_LOG(ArgusGameLog, Display, TEXT("[%s] Move To input executed while %d entities were selected."), ARGUS_FUNCNAME, numSelectedEntities);
	}
}
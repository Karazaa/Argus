// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusActor.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusPlayerController.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"

void UArgusInputManager::SetupInputComponent(TWeakObjectPtr<AArgusPlayerController> owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet)
{
	if (!owningPlayerController.IsValid())
	{
		UE_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] %s is setting up an InputComponent without a valid owning %s"),
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
			ArgusInputLog, Error, TEXT("[%s] Null input component, %s, assigned to %s."),
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
			ArgusInputLog, Error, TEXT("[%s] Failed to cast input component, %s, to a %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_owningPlayerController->InputComponent),
			ARGUS_NAMEOF(UEnhancedInputComponent)
		);
		return;
	}

	const UArgusInputActionSet* actionSet = argusInputActionSet.LoadSynchronous();
	if (!actionSet)
	{
		UE_LOG(ArgusInputLog, Error, TEXT("[%s] Failed to retrieve input action set, %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(argusInputActionSet));
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
		UE_LOG(ArgusInputLog, Error, TEXT("[%s] Null %s, assigned in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController), ARGUS_NAMEOF(UArgusInputManager));
		return false;
	}

	return true;
}

void UArgusInputManager::ProcessInputEvent(InputType inputType)
{
	ARGUS_TRACE(UArgusInputManager::ProcessInputEvent)

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

	AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor());
	if (!argusActor)
	{
		return;
	}

	if (isAdditive)
	{
		AddSelectedActorAdditive(argusActor);
	}
	else
	{
		AddSelectedActorExclusive(argusActor);
	}

	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		UE_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] selected an %s with ID %d. Is additive? %s"),
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

	FVector targetLocation = hitResult.Location;
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		UE_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Move To input occurred. Mouse projection worldspace location is (%f, %f, %f)"),
			ARGUS_FUNCNAME,
			targetLocation.X,
			targetLocation.Y,
			targetLocation.Z
		);
	}

	ETask inputTask = ETask::ProcessMoveToLocationCommand;
	ArgusEntity targetEntity = ArgusEntity::s_emptyEntity;
	if (AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor()))
	{
		targetEntity = argusActor->GetEntity();
		if (targetEntity && targetEntity.GetComponent<TransformComponent>())
		{
			inputTask = ETask::ProcessMoveToEntityCommand;
		}
	}

	const uint32 numSelectedActors = m_selectedArgusActors.Num();
	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		ProcessMoveToInputEventPerSelectedActor(selectedActor.Get(), inputTask, targetEntity, targetLocation);
	}
}

void UArgusInputManager::ProcessMoveToInputEventPerSelectedActor(AArgusActor* argusActor, ETask inputTask, ArgusEntity targetEntity, FVector targetLocation)
{
	ArgusEntity selectedEntity = argusActor->GetEntity();
	if (!selectedEntity)
	{
		return;
	}

	TaskComponent* taskComponent = selectedEntity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = selectedEntity.GetComponent<TargetingComponent>();

	if (!taskComponent || !targetingComponent)
	{
		return;
	}

	if (inputTask == ETask::ProcessMoveToEntityCommand)
	{
		if (targetEntity == selectedEntity)
		{
			taskComponent->m_currentTask = ETask::None;
		}
		else
		{
			taskComponent->m_currentTask = inputTask;
			targetingComponent->m_targetEntityId = targetEntity.GetId();
			targetingComponent->m_targetLocation.Reset();
		}
	}
	else if (inputTask == ETask::ProcessMoveToLocationCommand)
	{
		taskComponent->m_currentTask = inputTask;
		targetingComponent->m_targetEntityId = ArgusEntity::s_emptyEntity.GetId();
		targetingComponent->m_targetLocation = targetLocation;
	}
}

void UArgusInputManager::AddSelectedActorExclusive(AArgusActor* argusActor)
{
	if (!argusActor)
	{
		return;
	}

	bool alreadySelected = false;
	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (selectedActor.Get() == argusActor)
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

void UArgusInputManager::AddSelectedActorAdditive(AArgusActor* argusActor)
{
	if (!argusActor)
	{
		return;
	}

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

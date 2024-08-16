// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusActor.h"
#include "ArgusCameraActor.h"
#include "ArgusInputActionSet.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusPlayerController.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "Systems/TransformSystems.h"

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

#pragma region Input Action Delegates
void UArgusInputManager::OnSelect(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Select, value));
}

void UArgusInputManager::OnSelectAdditive(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SelectAdditive, value));
}

void UArgusInputManager::OnMarqueeSelect(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::MarqueeSelect, value));
}

void UArgusInputManager::OnMarqueeSelectAdditive(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::MarqueeSelectAdditive, value));
}

void UArgusInputManager::OnMoveTo(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::MoveTo, value));
}

void UArgusInputManager::OnSetWaypoint(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetWaypoint, value));
}

void UArgusInputManager::OnZoom(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Zoom, value));
}
#pragma endregion

void UArgusInputManager::ProcessPlayerInput(TObjectPtr<AArgusCameraActor>& argusCamera, const AArgusCameraActor::UpdateCameraPanningParameters& updateCameraParameters, float deltaTime)
{
	ARGUS_TRACE(UArgusInputManager::ProcessPlayerInput)

	const int inputsEventsThisFrameCount = m_inputEventsThisFrame.Num();
	for (int i = 0; i < inputsEventsThisFrameCount; ++i)
	{
		ProcessInputEvent(argusCamera, m_inputEventsThisFrame[i]);
	}
	m_inputEventsThisFrame.Empty();

	if (!argusCamera)
	{
		UE_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Did not recieve valid reference to %s. Unable to call %s as a result."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(AArgusCameraActor),
			ARGUS_NAMEOF(AArgusCameraActor::UpdateCameraPanning)
		);
		return;
	}

	argusCamera->UpdateCameraPanning(updateCameraParameters, deltaTime);
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

	if (const UInputAction* marqueeSelectAction = actionSet->m_marqueeSelectAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(marqueeSelectAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnMarqueeSelect);
	}

	if (const UInputAction* marqueeSelectAdditiveAction = actionSet->m_marqueeSelectAdditiveAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(marqueeSelectAdditiveAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnMarqueeSelectAdditive);
	}

	if (const UInputAction* moveToAction = actionSet->m_moveToAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(moveToAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnMoveTo);
	}

	if (const UInputAction* setWaypointAction = actionSet->m_setWaypointAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setWaypointAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetWaypoint);
	}

	if (const UInputAction* zoomAction = actionSet->m_zoomAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(zoomAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnZoom);
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

void UArgusInputManager::ProcessInputEvent(TObjectPtr<AArgusCameraActor>& argusCamera, const InputCache& inputType)
{
	ARGUS_TRACE(UArgusInputManager::ProcessInputEvent)

	switch (inputType.m_type)
	{
		case InputType::Select:
			ProcessSelectInputEvent(false);
			break;
		case InputType::SelectAdditive:
			ProcessSelectInputEvent(true);
			break;
		case InputType::MarqueeSelect:
			ProcessMarqueeSelectInputEvent(false);
			break;
		case InputType::MarqueeSelectAdditive:
			ProcessMarqueeSelectInputEvent(true);
			break;
		case InputType::MoveTo:
			ProcessMoveToInputEvent();
			break;
		case InputType::SetWaypoint:
			ProcessSetWaypointInputEvent();
			break;
		case InputType::Zoom:
			ProcessZoomInpuEvent(argusCamera, inputType.m_value);
			break;
		default:
			break;
	}
}

#pragma region Input Event Processing
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
	m_cachedLastSelectInputWorldspaceLocation = hitResult.Location;

	AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor());
	if (!argusActor)
	{
		return;
	}

	if (!m_owningPlayerController->IsArgusActorOnPlayerTeam(argusActor))
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

void UArgusInputManager::ProcessMarqueeSelectInputEvent(bool isAdditive)
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

	const FVector2D minXY = FVector2D
	(
		FMath::Min(hitResult.Location.X, m_cachedLastSelectInputWorldspaceLocation.X), 
		FMath::Min(hitResult.Location.Y, m_cachedLastSelectInputWorldspaceLocation.Y)
	);
	const FVector2D maxXY = FVector2D
	(
		FMath::Max(hitResult.Location.X, m_cachedLastSelectInputWorldspaceLocation.X),
		FMath::Max(hitResult.Location.Y, m_cachedLastSelectInputWorldspaceLocation.Y)
	);

	TArray<ArgusEntity> entitiesWithinBounds;
	TArray<AArgusActor*> actorsWithinBounds;
	TransformSystems::FindEntitiesWithinXYBounds(minXY, maxXY, entitiesWithinBounds);

	if (!m_owningPlayerController->GetArgusActorsFromArgusEntities(entitiesWithinBounds, actorsWithinBounds))
	{
		return;
	}
	m_owningPlayerController->FilterArgusActorsToPlayerTeam(actorsWithinBounds);
	
	const int numFoundEntities = actorsWithinBounds.Num();
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		UE_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Did a Marquee Select from {%f, %f} to {%f, %f}. Found %d entities. Is additive? %s"),
			ARGUS_FUNCNAME,
			minXY.X, minXY.Y,
			maxXY.X, maxXY.Y,
			numFoundEntities,
			isAdditive ? TEXT("Yes") : TEXT("No")
		);
	}

	if (!isAdditive && numFoundEntities > 0)
	{
		AddMarqueeSelectedActorsExclusive(actorsWithinBounds);
	}
	else
	{
		AddMarqueeSelectedActorsAdditive(actorsWithinBounds);
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
	if (!argusActor)
	{
		return;
	}

	ArgusEntity selectedEntity = argusActor->GetEntity();
	if (!selectedEntity)
	{
		return;
	}

	TaskComponent* taskComponent = selectedEntity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = selectedEntity.GetComponent<TargetingComponent>();
	NavigationComponent* navigationComponent = selectedEntity.GetComponent<NavigationComponent>();

	if (!taskComponent || !targetingComponent || !navigationComponent)
	{
		return;
	}

	navigationComponent->ResetQueuedWaypoints();
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

void UArgusInputManager::ProcessSetWaypointInputEvent()
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
			ArgusInputLog, Display, TEXT("[%s] Set Waypoint input occurred. Mouse projection worldspace location is (%f, %f, %f)"),
			ARGUS_FUNCNAME,
			targetLocation.X,
			targetLocation.Y,
			targetLocation.Z
		);
	}

	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		ProcessSetWaypointInputEventPerSelectedActor(selectedActor.Get(), targetLocation);
	}
}

void UArgusInputManager::ProcessSetWaypointInputEventPerSelectedActor(AArgusActor* argusActor, FVector targetLocation)
{
	if (!argusActor)
	{
		return;
	}

	ArgusEntity selectedEntity = argusActor->GetEntity();
	if (!selectedEntity)
	{
		return;
	}

	TaskComponent* taskComponent = selectedEntity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = selectedEntity.GetComponent<TargetingComponent>();
	NavigationComponent* navigationComponent = selectedEntity.GetComponent<NavigationComponent>();

	if (!taskComponent || !targetingComponent || !navigationComponent)
	{
		return;
	}

	switch (taskComponent->m_currentTask)
	{
		case ETask::None:
		case ETask::FailedToFindPath:
		case ETask::ProcessMoveToEntityCommand:
		case ETask::MoveToEntity:
			taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
			targetingComponent->m_targetEntityId = ArgusEntity::s_emptyEntity.GetId();
			targetingComponent->m_targetLocation = targetLocation;
			navigationComponent->ResetQueuedWaypoints();
			break;
		case ETask::ProcessMoveToLocationCommand:
		case ETask::MoveToLocation:
			taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
			navigationComponent->m_queuedWaypoints.push(targetLocation);
			break;
		default:
			break;
	}
}

void UArgusInputManager::ProcessZoomInpuEvent(TObjectPtr<AArgusCameraActor>& argusCamera, const FInputActionValue& value)
{
	const float zoomValue = value.Get<float>();
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		UE_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Zoom with a value of %f occurred."),
			ARGUS_FUNCNAME,
			zoomValue
		);
	}

	if (!argusCamera)
	{
		UE_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Did not recieve a valid reference to %s. Cannot call %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(AArgusCameraActor),
			ARGUS_NAMEOF(AArgusCameraActor::UpdateCameraZoom)
		);
		return;
	}

	argusCamera->UpdateCameraZoom(zoomValue);
}
#pragma endregion

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

void UArgusInputManager::AddMarqueeSelectedActorsExclusive(const TArray<AArgusActor*>& marqueeSelectedActors)
{
	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (selectedActor.IsValid())
		{
			selectedActor->SetSelectionState(false);
		}
	}
	m_selectedArgusActors.Empty();

	AddMarqueeSelectedActorsAdditive(marqueeSelectedActors);
}

void UArgusInputManager::AddMarqueeSelectedActorsAdditive(const TArray<AArgusActor*>& marqueeSelectedActors)
{
	const uint32 selectedActorsNum = marqueeSelectedActors.Num();
	const uint32 existingSelectedActorNum = m_selectedArgusActors.Num();
	m_selectedArgusActors.Reserve(selectedActorsNum + existingSelectedActorNum);

	for (uint32 i = 0u; i < selectedActorsNum; ++i)
	{
		if (marqueeSelectedActors[i])
		{
			marqueeSelectedActors[i]->SetSelectionState(true);
			m_selectedArgusActors.Emplace(marqueeSelectedActors[i]);
		}
	}
}

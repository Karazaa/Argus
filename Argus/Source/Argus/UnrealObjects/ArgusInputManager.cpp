// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusActor.h"
#include "ArgusCameraActor.h"
#include "ArgusInputActionSet.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusPlayerController.h"
#include "ArgusStaticData.h"
#include "ArgusTesting.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "Systems/TransformSystems.h"

#define ECC_RETICLE	ECC_GameTraceChannel1

void UArgusInputManager::SetupInputComponent(AArgusPlayerController* owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet)
{
	if (!owningPlayerController)
	{
		ARGUS_LOG
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
		ARGUS_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Null input component, %s, assigned to %s."),
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(m_owningPlayerController->InputComponent), 
			ARGUS_NAMEOF(m_owningPlayerController)
		);
		return;
	}

	UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(m_owningPlayerController->InputComponent.Get());
	UEnhancedPlayerInput* enhancedInput = Cast<UEnhancedPlayerInput>(m_owningPlayerController->PlayerInput);
	BindActions(argusInputActionSet, enhancedInputComponent, enhancedInput);
}

#pragma region Input Action Delegates
void UArgusInputManager::OnSelect(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Select, value));
}

void UArgusInputManager::OnSelectAdditive(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SelectAdditive, value));
}

void UArgusInputManager::OnMarqueeSelect(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::MarqueeSelect, value));
}

void UArgusInputManager::OnMarqueeSelectAdditive(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::MarqueeSelectAdditive, value));
}

void UArgusInputManager::OnMoveTo(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::MoveTo, value));
}

void UArgusInputManager::OnSetWaypoint(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetWaypoint, value));
}

void UArgusInputManager::OnZoom(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Zoom, value));
}

void UArgusInputManager::OnAbility0(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability0, value));
}

void UArgusInputManager::OnAbility1(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability1, value));
}

void UArgusInputManager::OnAbility2(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability2, value));
}

void UArgusInputManager::OnAbility3(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability3, value));
}

void UArgusInputManager::OnEscape(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Escape, value));
}

void UArgusInputManager::OnRotateCamera(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::RotateCamera, value));
}

void UArgusInputManager::OnStartPanningLockout(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::StartPanningLockout, value));
}

void UArgusInputManager::OnStopPanningLockout(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::StopPanningLockout, value));
}

void UArgusInputManager::OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent)
{
	switch (buttonClickedEvent)
	{
		case UArgusUIButtonClickedEventsEnum::SelectedArgusEntityAbility0:
			m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability0, FInputActionValue()));
			break;
		case UArgusUIButtonClickedEventsEnum::SelectedArgusEntityAbility1:
			m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability1, FInputActionValue()));
			break;
		case UArgusUIButtonClickedEventsEnum::SelectedArgusEntityAbility2:
			m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability2, FInputActionValue()));
			break;
		case UArgusUIButtonClickedEventsEnum::SelectedArgusEntityAbility3:
			m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability3, FInputActionValue()));
			break;
		default:
			break;
	}
}

#pragma endregion

void UArgusInputManager::ProcessPlayerInput(AArgusCameraActor* argusCamera, const AArgusCameraActor::UpdateCameraPanningParameters& updateCameraParameters, float deltaTime)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	ARGUS_TRACE(UArgusInputManager::ProcessPlayerInput);

#if WITH_AUTOMATION_TESTS
	if (ArgusTesting::IsInTestingContext())
	{
		m_inputEventsThisFrame.Empty();
		return;
	}
#endif // WITH_AUTOMATION_TESTS

	PrepareToProcessInputEvents();
	SetReticleState();

	const int inputsEventsThisFrameCount = m_inputEventsThisFrame.Num();
	for (int i = 0; i < inputsEventsThisFrameCount; ++i)
	{
		ProcessInputEvent(argusCamera, m_inputEventsThisFrame[i]);
	}
	m_inputEventsThisFrame.Empty();

	if (!argusCamera)
	{
		ARGUS_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Did not recieve valid reference to %s. Unable to call %s as a result."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(AArgusCameraActor),
			ARGUS_NAMEOF(AArgusCameraActor::UpdateCamera)
		);
		return;
	}

	argusCamera->UpdateCamera(updateCameraParameters, deltaTime);
}

bool UArgusInputManager::ShouldUpdateSelectedActorDisplay(ArgusEntity& templateSelectedEntity) const
{
	if (m_selectedArgusActorsChangedThisFrame)
	{
		if (m_activeAbilityGroupArgusActors.IsEmpty())
		{
			templateSelectedEntity = ArgusEntity::k_emptyEntity;
		}
		else if (AArgusActor* templateSelectedActor = (*m_activeAbilityGroupArgusActors.begin()).Get())
		{
			templateSelectedEntity = templateSelectedActor->GetEntity();
		}
		else
		{
			templateSelectedEntity = ArgusEntity::k_emptyEntity;
		}
		
		return true;
	}

	for (const TWeakObjectPtr<AArgusActor>& selectedActor : m_activeAbilityGroupArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		ArgusEntity selectedEntity = selectedActor->GetEntity();
		if (!selectedEntity)
		{
			continue;
		}

		TaskComponent* selectedTaskComponent = selectedEntity.GetComponent<TaskComponent>();
		if (!selectedTaskComponent)
		{
			continue;
		}

		if (selectedTaskComponent->m_constructionState == ConstructionState::ConstructionFinished)
		{
			templateSelectedEntity = selectedEntity;
			return true;
		}
	}

	return false;
}

bool UArgusInputManager::ShouldDrawMarqueeBox() const
{
	if (!m_selectInputDown)
	{
		return false;
	}

	if (ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity())
	{
		if (const ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>())
		{
			if (reticleComponent->IsReticleEnabled())
			{
				return false;
			}
		}
	}

	return true;
}

const FVector& UArgusInputManager::GetSelectionStartWorldSpaceLocation() const
{
	return m_cachedLastSelectInputWorldSpaceLocation;
}

const FVector2D UArgusInputManager::GetSelectionStartScreenSpaceLocation() const
{
	FVector2D output = FVector2D::ZeroVector;
	if (!m_owningPlayerController.IsValid())
	{
		return output;
	}
	
	if (!m_owningPlayerController->ProjectWorldLocationToScreen(m_cachedLastSelectInputWorldSpaceLocation, output))
	{
		return output;
	}

	return output;
}

void UArgusInputManager::BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, UEnhancedInputComponent* enhancedInputComponent, UEnhancedPlayerInput* enhancedInput)
{
	if (!enhancedInputComponent)
	{
		ARGUS_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Failed to cast input component, %s, to a %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_owningPlayerController->InputComponent),
			ARGUS_NAMEOF(UEnhancedInputComponent)
		);
		return;
	}
	if (!enhancedInput)
	{
		ARGUS_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Failed to cast player input, %s, to a %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_owningPlayerController->PlayerInput),
			ARGUS_NAMEOF(UEnhancedPlayerInput)
		);
		return;
	}

	const UArgusInputActionSet* actionSet = argusInputActionSet.LoadSynchronous();
	if (!actionSet)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Failed to retrieve input action set, %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(argusInputActionSet));
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
	if (const UInputAction* ability0Action = actionSet->m_ability0Action.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(ability0Action, ETriggerEvent::Triggered, this, &UArgusInputManager::OnAbility0);
	}
	if (const UInputAction* ability1Action = actionSet->m_ability1Action.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(ability1Action, ETriggerEvent::Triggered, this, &UArgusInputManager::OnAbility1);
	}
	if (const UInputAction* ability2Action = actionSet->m_ability2Action.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(ability2Action, ETriggerEvent::Triggered, this, &UArgusInputManager::OnAbility2);
	}
	if (const UInputAction* ability3Action = actionSet->m_ability3Action.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(ability3Action, ETriggerEvent::Triggered, this, &UArgusInputManager::OnAbility3);
	}
	if (const UInputAction* escapeAction = actionSet->m_escapeAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(escapeAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnEscape);
	}
	if (const UInputAction* rotateCameraAction = actionSet->m_rotateCameraAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(rotateCameraAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnRotateCamera);
	}
	if (const UInputAction* startPanningLockoutAction = actionSet->m_startPanningLockoutAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(startPanningLockoutAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnStartPanningLockout);
	}
	if (const UInputAction* stopPanningLockoutAction = actionSet->m_stopPanningLockoutAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(stopPanningLockoutAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnStopPanningLockout);
	}
}

bool UArgusInputManager::ValidateOwningPlayerController()
{
	if (!m_owningPlayerController.IsValid())
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Null %s, assigned in %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(m_owningPlayerController), ARGUS_NAMEOF(UArgusInputManager));
		return false;
	}

	return true;
}

void UArgusInputManager::PrepareToProcessInputEvents()
{
	m_selectedArgusActorsChangedThisFrame = false;
}

void UArgusInputManager::ProcessInputEvent(AArgusCameraActor* argusCamera, const InputCache& inputType)
{
	ARGUS_TRACE(UArgusInputManager::ProcessInputEvent);

	switch (inputType.m_type)
	{
		case InputType::Select:
			ProcessSelectInputEvent(false);
			break;
		case InputType::SelectAdditive:
			ProcessSelectInputEvent(true);
			break;
		case InputType::MarqueeSelect:
			ProcessMarqueeSelectInputEvent(argusCamera, false);
			break;
		case InputType::MarqueeSelectAdditive:
			ProcessMarqueeSelectInputEvent(argusCamera, true);
			break;
		case InputType::MoveTo:
			InterruptReticleFromInputEvent();
			ProcessMoveToInputEvent();
			break;
		case InputType::SetWaypoint:
			InterruptReticleFromInputEvent();
			ProcessSetWaypointInputEvent();
			break;
		case InputType::Zoom:
			ProcessZoomInputEvent(argusCamera, inputType.m_value);
			break;
		case InputType::Ability0:
			ProcessAbilityInputEvent(0u);
			break;
		case InputType::Ability1:
			ProcessAbilityInputEvent(1u);
			break;
		case InputType::Ability2:
			ProcessAbilityInputEvent(2u);
			break;
		case InputType::Ability3:
			ProcessAbilityInputEvent(3u);
			break;
		case InputType::Escape:
			ProcessEscapeInputEvent();
			break;
		case InputType::RotateCamera:
			ProcessRotateCameraInputEvent(argusCamera, inputType.m_value);
			break;
		case InputType::StartPanningLockout:
			if (argusCamera)
			{
				m_canRotateCamera = true;
				argusCamera->IncrementPanningBlockers();
			}
			break;
		case InputType::StopPanningLockout:
			if (argusCamera)
			{
				m_canRotateCamera = false;
				argusCamera->DecrementPanningBlockers();
			}
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
	if (!m_owningPlayerController->GetMouseProjectionLocation(ECC_WorldStatic, hitResult))
	{
		return;
	}
	m_cachedLastSelectInputWorldSpaceLocation = hitResult.Location;
	m_selectInputDown = true;

	if (ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity())
	{
		if (const ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>())
		{
			if (reticleComponent->IsReticleEnabled())
			{
				return;
			}
		}
	}

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
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] selected an %s with ID %d. Is additive? %s"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(AArgusActor),
			argusActor->GetEntity().GetId(),
			isAdditive ? TEXT("Yes") : TEXT("No")
		);
	}
}

void UArgusInputManager::ProcessMarqueeSelectInputEvent(AArgusCameraActor* argusCamera, bool isAdditive)
{
	if (!ValidateOwningPlayerController() || !argusCamera)
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	FHitResult hitResult;
	if (!m_owningPlayerController->GetMouseProjectionLocation(ECC_WorldStatic, hitResult))
	{
		return;
	}
	m_selectInputDown = false;

	if (const ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>())
	{
		if (reticleComponent->IsReticleEnabled())
		{
			ProcessReticleAbilityForSelectedActors(reticleComponent);
			return;
		}
	}

	TArray<FVector2D> convexPolygon;
	convexPolygon.SetNumZeroed(4);
	const FVector2D panUpDirection = FVector2D(AArgusCameraActor::GetPanUpVector());
	const FVector2D panRightDirection = FVector2D(AArgusCameraActor::GetPanRightVector());
	const FVector2D cameraLocation = FVector2D(argusCamera->GetCameraLocationWithoutZoom());
	convexPolygon[0] = FVector2D(m_cachedLastSelectInputWorldSpaceLocation);
	convexPolygon[2] = FVector2D(hitResult.Location);
	const FVector2D fromCameraToFirstPoint = convexPolygon[0] - cameraLocation;
	const FVector2D fromCameraToLastPoint = convexPolygon[2] - cameraLocation;

	float dotFirstForward = fromCameraToFirstPoint.Dot(panUpDirection);
	float dotLastForward = fromCameraToLastPoint.Dot(panUpDirection);
	float dotFirstRight = fromCameraToFirstPoint.Dot(panRightDirection);
	float dotLastRight = fromCameraToLastPoint.Dot(panRightDirection);

	FVector2D generatedVerticalPoint = convexPolygon[0] + (panUpDirection * (dotLastForward - dotFirstForward));
	FVector2D generatedHorizontalPoint = convexPolygon[0] + (panRightDirection * (dotLastRight - dotFirstRight));
	if (dotFirstForward > dotLastForward)
	{
		// Down and to the left in screen space.
		if (dotFirstRight > dotLastRight)
		{
			convexPolygon[1] = generatedVerticalPoint;
			convexPolygon[3] = generatedHorizontalPoint;
		}
		// Down and to the right in screen space.
		else
		{
			convexPolygon[1] = generatedHorizontalPoint;
			convexPolygon[3] = generatedVerticalPoint;
		}
	}
	else
	{
		// Up and to the left in screen space.
		if (dotFirstRight > dotLastRight)
		{
			convexPolygon[1] = generatedHorizontalPoint;
			convexPolygon[3] = generatedVerticalPoint;
		}
		// Up and to the right in screen space.
		else
		{
			convexPolygon[1] = generatedVerticalPoint;
			convexPolygon[3] = generatedHorizontalPoint;
		}
	}

	TArray<uint16> entityIdsWithinBounds;
	spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinConvexPoly(entityIdsWithinBounds, convexPolygon);

	TArray<AArgusActor*> actorsWithinBounds;
	if (!m_owningPlayerController->GetArgusActorsFromArgusEntityIds(entityIdsWithinBounds, actorsWithinBounds))
	{
		return;
	}
	m_owningPlayerController->FilterArgusActorsToPlayerTeam(actorsWithinBounds);
	
	const int numFoundEntities = actorsWithinBounds.Num();
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Did a Marquee Select from {%f, %f} to {%f, %f}. Found %d entities. Is additive? %s"),
			ARGUS_FUNCNAME,
			convexPolygon[0].X, convexPolygon[0].Y,
			convexPolygon[2].X, convexPolygon[2].Y,
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

	if (m_canRotateCamera)
	{
		return;
	}

	FHitResult hitResult;
	if (!m_owningPlayerController->GetMouseProjectionLocation(ECC_WorldStatic, hitResult))
	{
		return;
	}

	FVector targetLocation = hitResult.Location;
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Move To input occurred. Mouse projection worldspace location is (%f, %f, %f)"),
			ARGUS_FUNCNAME,
			targetLocation.X,
			targetLocation.Y,
			targetLocation.Z
		);
	}

	MovementState inputMovementState = MovementState::ProcessMoveToLocationCommand;
	ArgusEntity targetEntity = ArgusEntity::k_emptyEntity;
	if (AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor()))
	{
		targetEntity = argusActor->GetEntity();
		if (targetEntity && targetEntity.GetComponent<TransformComponent>())
		{
			inputMovementState = MovementState::ProcessMoveToEntityCommand;
		}
	}

	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		ProcessMoveToInputEventPerSelectedActor(selectedActor.Get(), inputMovementState, targetEntity, targetLocation);
	}
}

void UArgusInputManager::ProcessMoveToInputEventPerSelectedActor(AArgusActor* argusActor, MovementState inputMovementState, ArgusEntity targetEntity, FVector targetLocation)
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

	if (!taskComponent || !targetingComponent)
	{
		return;
	}

	if (navigationComponent)
	{
		navigationComponent->ResetQueuedWaypoints();
	}
	
	if (inputMovementState == MovementState::ProcessMoveToEntityCommand)
	{
		if (targetEntity == selectedEntity)
		{
			taskComponent->m_movementState = MovementState::None;
		}
		else
		{
			if (navigationComponent)
			{
				taskComponent->m_movementState = inputMovementState;
			}
			
			targetingComponent->m_targetEntityId = targetEntity.GetId();
			targetingComponent->m_targetLocation.Reset();
		}
	}
	else if (inputMovementState == MovementState::ProcessMoveToLocationCommand)
	{
		if (navigationComponent)
		{
			taskComponent->m_movementState = inputMovementState;
		}
		
		targetingComponent->m_targetEntityId = ArgusEntity::k_emptyEntity.GetId();
		targetingComponent->m_targetLocation = targetLocation;
	}
}

void UArgusInputManager::ProcessSetWaypointInputEvent()
{
	if (!ValidateOwningPlayerController())
	{
		return;
	}

	if (m_canRotateCamera)
	{
		return;
	}

	FHitResult hitResult;
	if (!m_owningPlayerController->GetMouseProjectionLocation(ECC_WorldStatic, hitResult))
	{
		return;
	}

	FVector targetLocation = hitResult.Location;
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
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
	ARGUS_MEMORY_TRACE(ArgusNavigationSystems);

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

	switch (taskComponent->m_movementState)
	{
		case MovementState::None:
		case MovementState::FailedToFindPath:
		case MovementState::ProcessMoveToEntityCommand:
		case MovementState::MoveToEntity:
			taskComponent->m_movementState = MovementState::ProcessMoveToLocationCommand;
			targetingComponent->m_targetEntityId = ArgusEntity::k_emptyEntity.GetId();
			targetingComponent->m_targetLocation = targetLocation;
			navigationComponent->ResetQueuedWaypoints();
			break;
		case MovementState::ProcessMoveToLocationCommand:
		case MovementState::MoveToLocation:
			taskComponent->m_movementState = MovementState::ProcessMoveToLocationCommand;
			navigationComponent->m_queuedWaypoints.Enqueue(targetLocation);
			break;
		default:
			break;
	}
}

void UArgusInputManager::ProcessZoomInputEvent(AArgusCameraActor* argusCamera, const FInputActionValue& value)
{
	const float zoomValue = value.Get<float>();
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Zoom with a value of %f occurred."),
			ARGUS_FUNCNAME,
			zoomValue
		);
	}

	if (!argusCamera)
	{
		ARGUS_LOG
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

void UArgusInputManager::ProcessAbilityInputEvent(uint8 abilityIndex)
{
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Pressed Ability %d."),
			ARGUS_FUNCNAME,
			abilityIndex
		);
	}

	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_activeAbilityGroupArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		ProcessAbilityInputEventPerSelectedActor(selectedActor.Get(), abilityIndex);
	}
}

void UArgusInputManager::ProcessAbilityInputEventPerSelectedActor(AArgusActor* argusActor, uint8 abilityIndex)
{
	if (!argusActor)
	{
		return;
	}

	ArgusEntity entity = argusActor->GetEntity();
	if (!entity)
	{
		return;
	}

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	switch (abilityIndex)
	{
		case 0u:
			taskComponent->m_abilityState = AbilityState::ProcessCastAbility0Command;
			break;
		case 1u:
			taskComponent->m_abilityState = AbilityState::ProcessCastAbility1Command;
			break;
		case 2u:
			taskComponent->m_abilityState = AbilityState::ProcessCastAbility2Command;
			break;
		case 3u:
			taskComponent->m_abilityState = AbilityState::ProcessCastAbility3Command;
			break;
		default:
			break;
	}
}

void UArgusInputManager::ProcessEscapeInputEvent()
{
	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		ARGUS_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Could not retrieve a valid %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(singletonEntity)
		);
		return;
	}

	ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>();
	if (!reticleComponent)
	{
		ARGUS_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Could not retrieve a valid %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ReticleComponent*)
		);
		return;
	}

	if (reticleComponent->IsReticleEnabled())
	{
		reticleComponent->DisableReticle();
	}
;}

void UArgusInputManager::ProcessRotateCameraInputEvent(AArgusCameraActor* argusCamera, const FInputActionValue& value)
{
	const float rotationValue = value.Get<float>();
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Rotate Camera with a value of %f occurred."),
			ARGUS_FUNCNAME,
			rotationValue
		);
	}

	if (!argusCamera)
	{
		ARGUS_LOG
		(
			ArgusInputLog, Error, TEXT("[%s] Did not recieve a valid reference to %s. Cannot call %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(AArgusCameraActor),
			ARGUS_NAMEOF(AArgusCameraActor::UpdateCameraOrbit)
		);
		return;
	}

	argusCamera->UpdateCameraOrbit(rotationValue);
}

#pragma endregion

void UArgusInputManager::AddSelectedActorExclusive(AArgusActor* argusActor)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);

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

	OnSelectedArgusArgusActorsChanged();
}

void UArgusInputManager::AddSelectedActorAdditive(AArgusActor* argusActor)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);

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

	OnSelectedArgusArgusActorsChanged();
}

void UArgusInputManager::AddMarqueeSelectedActorsExclusive(const TArray<AArgusActor*>& marqueeSelectedActors)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);

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
	ARGUS_MEMORY_TRACE(ArgusInputManager);

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

	OnSelectedArgusArgusActorsChanged();
}

void UArgusInputManager::OnSelectedArgusArgusActorsChanged()
{
	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Could not retrieve %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(singletonEntity));
		return;
	}

	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Could not retrieve a valid %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(InputInterfaceComponent), ARGUS_NAMEOF(singletonEntity));
		return;
	}

	inputInterfaceComponent->m_selectedArgusEntityIds.Reset();
	inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Reset();
	m_activeAbilityGroupArgusActors.Reset();

	uint32 abilityUnitGroupActorRecordId = 0u;

	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		const ArgusEntity entity = selectedActor->GetEntity();
		if (!entity)
		{
			continue;
		}
		inputInterfaceComponent->m_selectedArgusEntityIds.Add(entity.GetId());

		const AbilityComponent* abilityComponent = entity.GetComponent<AbilityComponent>();
		if (!abilityComponent)
		{
			continue;
		}

		const TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
		if (!taskComponent)
		{
			continue;
		}

		if (abilityUnitGroupActorRecordId == 0u)
		{
			abilityUnitGroupActorRecordId = taskComponent->m_spawnedFromArgusActorRecordId;
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(entity.GetId());
			m_activeAbilityGroupArgusActors.Add(selectedActor);
		}
		else if (abilityUnitGroupActorRecordId == taskComponent->m_spawnedFromArgusActorRecordId)
		{
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(entity.GetId());
			m_activeAbilityGroupArgusActors.Add(selectedActor);
		}
	}

	m_selectedArgusActorsChangedThisFrame = true;
}

void UArgusInputManager::InterruptReticleFromInputEvent()
{
	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>();
	if (!reticleComponent)
	{
		return;
	}

	if (!reticleComponent->IsReticleEnabled())
	{
		return;
	}

	reticleComponent->DisableReticle();
}

void UArgusInputManager::SetReticleState()
{
	ARGUS_TRACE(UArgusInputManager::SetReticleState);

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>();
	if (!reticleComponent)
	{
		return;
	}

	if (!reticleComponent->IsReticleEnabled())
	{
		return;
	}

	if (reticleComponent->m_wasAbilityCast)
	{
		if (const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(reticleComponent->m_abilityRecordId))
		{
			if (abilityRecord->GetDisableReticleAfterCast())
			{
				reticleComponent->DisableReticle();
				return;
			}
			
			reticleComponent->m_wasAbilityCast = false;
		}
	}

	FHitResult hitResult;
	if (!m_owningPlayerController->GetMouseProjectionLocation(ECC_RETICLE, hitResult))
	{
		return;
	}

	reticleComponent->m_reticleLocation = hitResult.Location;

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	TArray<uint16> nearbyArgusEntityIds;
	const float querySize = reticleComponent->m_radius;
	spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(nearbyArgusEntityIds, reticleComponent->m_reticleLocation, querySize);
	bool anyFound = nearbyArgusEntityIds.Num() > 0;

	if (!anyFound)
	{
		TArray<ObstacleIndicies> obstacleIndicies;
		FVector location = ArgusMath::ToCartesianVector(reticleComponent->m_reticleLocation);
		location.Z = 0.0f;
		spatialPartitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation(obstacleIndicies, location, querySize);
		anyFound = obstacleIndicies.Num() > 0;
	}

	reticleComponent->m_isBlocked = anyFound;
}

void UArgusInputManager::ProcessReticleAbilityForSelectedActors(const ReticleComponent* reticleComponent)
{
	if (!reticleComponent)
	{
		return;
	}

	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Pressed select for Reticle Ability %d."),
			ARGUS_FUNCNAME,
			reticleComponent->m_abilityRecordId
		);
	}

	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_activeAbilityGroupArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		ProcessReticleAbilityPerSelectedActor(selectedActor.Get(), reticleComponent->m_abilityRecordId);
	}
}

void UArgusInputManager::ProcessReticleAbilityPerSelectedActor(AArgusActor* argusActor, uint32 abilityRecordId)
{
	if (!argusActor)
	{
		return;
	}

	ArgusEntity entity = argusActor->GetEntity();
	if (!entity)
	{
		return;
	}

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return;
	}

	AbilityComponent* abilityComponent = entity.GetComponent<AbilityComponent>();
	if (!abilityComponent)
	{
		return;
	}

	if (!abilityComponent->HasAbility(abilityRecordId))
	{
		return;
	}

	taskComponent->m_abilityState = AbilityState::ProcessCastReticleAbility;
}

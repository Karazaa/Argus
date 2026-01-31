// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusInputManager.h"
#include "ArgusActor.h"
#include "ArgusCameraActor.h"
#include "ArgusECSDebugger.h"
#include "ArgusInputActionSet.h"
#include "ArgusLogging.h"
#include "ArgusPlayerController.h"
#include "ArgusStaticData.h"
#include "ArgusTesting.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "Systems/AbilitySystems.h"
#include "Systems/DecalSystems.h"
#include "Systems/InputInterfaceSystems.h"
#include "Systems/SpatialPartitioningSystems.h"

bool UArgusInputManager::ShouldUpdateSelectedActorDisplay(ArgusEntity& templateSelectedEntity)
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusInputLog);

	if (inputInterfaceComponent->m_selectedActorsDisplayState == ESelectedActorsDisplayState::ChangedThisFrame)
	{
		if (inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.IsEmpty())
		{
			templateSelectedEntity = ArgusEntity::k_emptyEntity;
		}
		else
		{
			templateSelectedEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds[0]);
		}

		return true;
	}

	for (int32 i = 0; i < inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Num(); ++i)
	{
		ArgusEntity potentialTemplate = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds[0]);
		if (!potentialTemplate)
		{
			continue;
		}

		TaskComponent* potentialTemplateTaskComponent = potentialTemplate.GetComponent<TaskComponent>();
		if (!potentialTemplateTaskComponent)
		{
			continue;
		}

		if (potentialTemplateTaskComponent->m_constructionState == EConstructionState::ConstructionFinished)
		{
			templateSelectedEntity = potentialTemplate;
			return true;
		}
	}

	return false;
}

void UArgusInputManager::SetupInputComponent(AArgusPlayerController* owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet)
{
	ARGUS_RETURN_ON_NULL(owningPlayerController, ArgusInputLog);
	m_owningPlayerController = owningPlayerController;
	ARGUS_RETURN_ON_NULL(m_owningPlayerController->InputComponent, ArgusInputLog);

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

void UArgusInputManager::OnAttackMoveTo(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::AttackMoveTo, value));
}

void UArgusInputManager::OnSetWaypoint(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetWaypoint, value));
}

void UArgusInputManager::OnZoom(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Zoom, value));
}

void UArgusInputManager::OnAbility0(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability0, value));
}

void UArgusInputManager::OnAbility1(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability1, value));
}

void UArgusInputManager::OnAbility2(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability2, value));
}

void UArgusInputManager::OnAbility3(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Ability3, value));
}

void UArgusInputManager::OnEscape(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::Escape, value));
}

void UArgusInputManager::OnRotateCamera(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::RotateCamera, value));
}

void UArgusInputManager::OnStartPanningLockout(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::StartPanningLockout, value));
}

void UArgusInputManager::OnStopPanningLockout(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::StopPanningLockout, value));
}

void UArgusInputManager::OnUserInterfaceButtonClicked(InputType inputEvent)
{
	m_inputEventsThisFrame.Emplace(InputCache(inputEvent, FInputActionValue()));
}

void UArgusInputManager::OnUserInterfaceEntityClicked(ArgusEntity clickedEntity)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::UserInterfaceEntityClicked, clickedEntity));
}

void UArgusInputManager::OnUserInterfaceFocusEntityClicked(ArgusEntity clickedEntity)
{
	if (!ValidateOwningPlayerController())
	{
		return;
	}

	AArgusCameraActor* cameraActor = m_owningPlayerController->GetArgusCameraActor();
	ARGUS_RETURN_ON_NULL(cameraActor, ArgusInputLog);

	cameraActor->FocusOnArgusEntity(clickedEntity);
}

void UArgusInputManager::OnControlGroup0(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup0, value));
}

void UArgusInputManager::OnControlGroup1(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup1, value));
}

void UArgusInputManager::OnControlGroup2(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup2, value));
}

void UArgusInputManager::OnControlGroup3(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup3, value));
}

void UArgusInputManager::OnControlGroup4(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup4, value));
}

void UArgusInputManager::OnControlGroup5(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup5, value));
}

void UArgusInputManager::OnControlGroup6(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup6, value));
}

void UArgusInputManager::OnControlGroup7(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup7, value));
}

void UArgusInputManager::OnControlGroup8(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup8, value));
}

void UArgusInputManager::OnControlGroup9(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup9, value));
}

void UArgusInputManager::OnControlGroup10(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup10, value));
}

void UArgusInputManager::OnControlGroup11(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup11, value));
}

void UArgusInputManager::OnSetControlGroup0(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup0, value));
}

void UArgusInputManager::OnSetControlGroup1(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup1, value));
}

void UArgusInputManager::OnSetControlGroup2(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup2, value));
}

void UArgusInputManager::OnSetControlGroup3(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup3, value));
}

void UArgusInputManager::OnSetControlGroup4(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup4, value));
}

void UArgusInputManager::OnSetControlGroup5(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup5, value));
}

void UArgusInputManager::OnSetControlGroup6(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup6, value));
}

void UArgusInputManager::OnSetControlGroup7(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup7, value));
}

void UArgusInputManager::OnSetControlGroup8(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup8, value));
}

void UArgusInputManager::OnSetControlGroup9(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup9, value));
}

void UArgusInputManager::OnSetControlGroup10(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup10, value));
}

void UArgusInputManager::OnSetControlGroup11(const FInputActionValue& value)
{
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup11, value));
}

void UArgusInputManager::OnChangeActiveAbilityGroup(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ChangeActiveAbilityGroup, value));
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

	ARGUS_RETURN_ON_NULL(argusCamera, ArgusInputLog);

	argusCamera->UpdateCamera(updateCameraParameters, deltaTime);
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
	ARGUS_RETURN_ON_NULL(enhancedInputComponent, ArgusInputLog);
	ARGUS_RETURN_ON_NULL(enhancedInput, ArgusInputLog);

	const UArgusInputActionSet* actionSet = argusInputActionSet.LoadSynchronous();
	ARGUS_RETURN_ON_NULL(actionSet, ArgusInputLog);

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
	if (const UInputAction* attackMoveToAction = actionSet->m_attackMoveToAction.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(attackMoveToAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnAttackMoveTo);
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
	if (const UInputAction* controlGroupAction0 = actionSet->m_controlGroup0.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction0, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup0);
	}
	if (const UInputAction* controlGroupAction1 = actionSet->m_controlGroup1.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction1, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup1);
	}
	if (const UInputAction* controlGroupAction2 = actionSet->m_controlGroup2.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction2, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup2);
	}
	if (const UInputAction* controlGroupAction3 = actionSet->m_controlGroup3.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction3, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup3);
	}
	if (const UInputAction* controlGroupAction4 = actionSet->m_controlGroup4.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction4, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup4);
	}
	if (const UInputAction* controlGroupAction5 = actionSet->m_controlGroup5.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction5, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup5);
	}
	if (const UInputAction* controlGroupAction6 = actionSet->m_controlGroup6.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction6, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup6);
	}
	if (const UInputAction* controlGroupAction7 = actionSet->m_controlGroup7.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction7, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup7);
	}
	if (const UInputAction* controlGroupAction8 = actionSet->m_controlGroup8.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction8, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup8);
	}
	if (const UInputAction* controlGroupAction9 = actionSet->m_controlGroup9.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction9, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup9);
	}
	if (const UInputAction* controlGroupAction10 = actionSet->m_controlGroup10.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction10, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup10);
	}
	if (const UInputAction* controlGroupAction11 = actionSet->m_controlGroup11.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(controlGroupAction11, ETriggerEvent::Triggered, this, &UArgusInputManager::OnControlGroup11);
	}
	if (const UInputAction* setControlGroupAction0 = actionSet->m_setControlGroup0.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction0, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup0);
	}
	if (const UInputAction* setControlGroupAction1 = actionSet->m_setControlGroup1.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction1, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup1);
	}
	if (const UInputAction* setControlGroupAction2 = actionSet->m_setControlGroup2.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction2, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup2);
	}
	if (const UInputAction* setControlGroupAction3 = actionSet->m_setControlGroup3.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction3, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup3);
	}
	if (const UInputAction* setControlGroupAction4 = actionSet->m_setControlGroup4.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction4, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup4);
	}
	if (const UInputAction* setControlGroupAction5 = actionSet->m_setControlGroup5.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction5, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup5);
	}
	if (const UInputAction* setControlGroupAction6 = actionSet->m_setControlGroup6.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction6, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup6);
	}
	if (const UInputAction* setControlGroupAction7 = actionSet->m_setControlGroup7.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction7, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup7);
	}
	if (const UInputAction* setControlGroupAction8 = actionSet->m_setControlGroup8.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction8, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup8);
	}
	if (const UInputAction* setControlGroupAction9 = actionSet->m_setControlGroup9.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction9, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup9);
	}
	if (const UInputAction* setControlGroupAction10 = actionSet->m_setControlGroup10.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction10, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup10);
	}
	if (const UInputAction* setControlGroupAction11 = actionSet->m_setControlGroup11.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(setControlGroupAction11, ETriggerEvent::Triggered, this, &UArgusInputManager::OnSetControlGroup11);
	}
	if (const UInputAction* changeActiveAbilityGroupAction = actionSet->m_changeActiveAbilityGroup.LoadSynchronous())
	{
		enhancedInputComponent->BindAction(changeActiveAbilityGroupAction, ETriggerEvent::Triggered, this, &UArgusInputManager::OnChangeActiveAbilityGroup);
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
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusInputLog);

	inputInterfaceComponent->m_selectedActorsDisplayState = ESelectedActorsDisplayState::NotChanged;
	InputInterfaceSystems::RemoveNoLongerSelectableEntities();
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
			InputInterfaceSystems::InterruptReticle();
			ProcessMoveToInputEvent(false);
			break;
		case InputType::AttackMoveTo:
			InputInterfaceSystems::InterruptReticle();
			ProcessMoveToInputEvent(true);
			break;
		case InputType::SetWaypoint:
			InputInterfaceSystems::InterruptReticle();
			ProcessSetWaypointInputEvent();
			break;
		case InputType::Zoom:
			ProcessZoomInputEvent(argusCamera, inputType.m_value);
			break;
		case InputType::Ability0:
			ProcessAbilityInputEvent(EAbilityIndex::Ability0);
			break;
		case InputType::Ability1:
			ProcessAbilityInputEvent(EAbilityIndex::Ability1);
			break;
		case InputType::Ability2:
			ProcessAbilityInputEvent(EAbilityIndex::Ability2);
			break;
		case InputType::Ability3:
			ProcessAbilityInputEvent(EAbilityIndex::Ability3);
			break;
		case InputType::Escape:
			InputInterfaceSystems::InterruptReticle();
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
		case InputType::ControlGroup0:
			ProcessControlGroup(0u, argusCamera);
			break;
		case InputType::ControlGroup1:
			ProcessControlGroup(1u, argusCamera);
			break;
		case InputType::ControlGroup2:
			ProcessControlGroup(2u, argusCamera);
			break;
		case InputType::ControlGroup3:
			ProcessControlGroup(3u, argusCamera);
			break;
		case InputType::ControlGroup4:
			ProcessControlGroup(4u, argusCamera);
			break;
		case InputType::ControlGroup5:
			ProcessControlGroup(5u, argusCamera);
			break;
		case InputType::ControlGroup6:
			ProcessControlGroup(6u, argusCamera);
			break;
		case InputType::ControlGroup7:
			ProcessControlGroup(7u, argusCamera);
			break;
		case InputType::ControlGroup8:
			ProcessControlGroup(8u, argusCamera);
			break;
		case InputType::ControlGroup9:
			ProcessControlGroup(9u, argusCamera);
			break;
		case InputType::ControlGroup10:
			ProcessControlGroup(10u, argusCamera);
			break;
		case InputType::ControlGroup11:
			ProcessControlGroup(11u, argusCamera);
			break;
		case InputType::SetControlGroup0:
			ProcessSetControlGroup(0u);
			break;
		case InputType::SetControlGroup1:
			ProcessSetControlGroup(1u);
			break;
		case InputType::SetControlGroup2:
			ProcessSetControlGroup(2u);
			break;
		case InputType::SetControlGroup3:
			ProcessSetControlGroup(3u);
			break;
		case InputType::SetControlGroup4:
			ProcessSetControlGroup(4u);
			break;
		case InputType::SetControlGroup5:
			ProcessSetControlGroup(5u);
			break;
		case InputType::SetControlGroup6:
			ProcessSetControlGroup(6u);
			break;
		case InputType::SetControlGroup7:
			ProcessSetControlGroup(7u);
			break;
		case InputType::SetControlGroup8:
			ProcessSetControlGroup(8u);
			break;
		case InputType::SetControlGroup9:
			ProcessSetControlGroup(9u);
			break;
		case InputType::SetControlGroup10:
			ProcessSetControlGroup(10u);
			break;
		case InputType::SetControlGroup11:
			ProcessSetControlGroup(11u);
			break;
		case InputType::ChangeActiveAbilityGroup:
			InputInterfaceSystems::InterruptReticle();
			ProcessChangeActiveAbilityGroup();
			break;
		case InputType::UserInterfaceEntityClicked:
			ProcessUserInterfaceEntityClicked(inputType.m_entity);
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

	bool shouldIgnoreTeamRequirement = false;

#if !UE_BUILD_SHIPPING
	shouldIgnoreTeamRequirement = ArgusECSDebugger::ShouldIgnoreTeamRequirementsForSelectingEntities();
#endif //!UE_BUILD_SHIPPING

	if (!shouldIgnoreTeamRequirement && !m_owningPlayerController->IsArgusActorOnPlayerTeam(argusActor))
	{
		return;
	}

	if (!shouldIgnoreTeamRequirement && (!argusActor->GetEntity().IsAlive() || argusActor->GetEntity().IsPassenger()))
	{
		return;
	}

	if (isAdditive)
	{
		InputInterfaceSystems::AddSelectedEntityAdditive(argusActor->GetEntity(), m_owningPlayerController->GetMoveToLocationDecalActorRecord());
	}
	else
	{
		InputInterfaceSystems::AddSelectedEntityExclusive(argusActor->GetEntity(), m_owningPlayerController->GetMoveToLocationDecalActorRecord());
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

void UArgusInputManager::ProcessMarqueeSelectInputEvent(const AArgusCameraActor* argusCamera, bool isAdditive)
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

	SpatialPartitioningComponent* spatialPartitioningComponent = singletonEntity.GetComponent<SpatialPartitioningComponent>();
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
			ProcessReticleAbilityForSelectedEntities(reticleComponent);
			return;
		}
	}

	if (FVector::DistSquared(m_cachedLastSelectInputWorldSpaceLocation, hitResult.Location) < FMath::Square(ArgusECSConstants::k_minimumMarqueeSelectDistance))
	{
		return;
	}

	TArray<FVector2D> groundConvexPolygon;
	TArray<FVector2D> flyingConvexPolygon;
	groundConvexPolygon.SetNumZeroed(4);
	flyingConvexPolygon.SetNumZeroed(4);
	groundConvexPolygon[0] = FVector2D(m_cachedLastSelectInputWorldSpaceLocation);
	groundConvexPolygon[2] = FVector2D(hitResult.Location);

	FVector direction0 = argusCamera->GetActorLocation() - m_cachedLastSelectInputWorldSpaceLocation;
	FVector direction2 = argusCamera->GetActorLocation() - hitResult.Location;
	direction0 *= ArgusMath::SafeDivide(1.0f, direction0.Z, 0.0f) * spatialPartitioningComponent->m_flyingPlaneHeight;
	direction2 *= ArgusMath::SafeDivide(1.0f, direction2.Z, 0.0f) * spatialPartitioningComponent->m_flyingPlaneHeight;
	flyingConvexPolygon[0] = FVector2D(m_cachedLastSelectInputWorldSpaceLocation + direction0);
	flyingConvexPolygon[2] = FVector2D(hitResult.Location + direction2);

	PopulateMarqueeSelectPolygon(argusCamera, groundConvexPolygon);
	PopulateMarqueeSelectPolygon(argusCamera, flyingConvexPolygon);

	TArray<uint16> entityIdsWithinBounds;
	spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinConvexPoly(entityIdsWithinBounds, groundConvexPolygon);

	TArray<uint16> flyingEntityIdsWithinBounds;
	spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindArgusEntityIdsWithinConvexPoly(flyingEntityIdsWithinBounds, flyingConvexPolygon);

	entityIdsWithinBounds.Reserve(entityIdsWithinBounds.Num() + flyingEntityIdsWithinBounds.Num());
	for (int32 i = 0; i < flyingEntityIdsWithinBounds.Num(); ++i)
	{
		entityIdsWithinBounds.Add(flyingEntityIdsWithinBounds[i]);
	}

	bool shouldIgnoreTeamRequirement = false;

#if !UE_BUILD_SHIPPING
	shouldIgnoreTeamRequirement = ArgusECSDebugger::ShouldIgnoreTeamRequirementsForSelectingEntities();
#endif //!UE_BUILD_SHIPPING

	if (!shouldIgnoreTeamRequirement)
	{
		m_owningPlayerController->FilterArgusEntityIdsToPlayerTeam(entityIdsWithinBounds);
	}
	
	const int numFoundEntities = entityIdsWithinBounds.Num();
	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Did a Marquee Select from {%f, %f} to {%f, %f}. Found %d entities. Is additive? %s"),
			ARGUS_FUNCNAME,
			groundConvexPolygon[0].X, groundConvexPolygon[0].Y,
			groundConvexPolygon[2].X, groundConvexPolygon[2].Y,
			numFoundEntities,
			isAdditive ? TEXT("Yes") : TEXT("No")
		);
	}

	if (!isAdditive && numFoundEntities > 0)
	{
		InputInterfaceSystems::AddMultipleSelectedEntitiesExclusive(entityIdsWithinBounds, m_owningPlayerController->GetMoveToLocationDecalActorRecord());
	}
	else
	{
		InputInterfaceSystems::AddMultipleSelectedEntitiesAdditive(entityIdsWithinBounds, m_owningPlayerController->GetMoveToLocationDecalActorRecord());
	}
}

void UArgusInputManager::PopulateMarqueeSelectPolygon(const AArgusCameraActor* argusCamera, TArray<FVector2D>& convexPolygon)
{
	ARGUS_RETURN_ON_NULL(argusCamera, ArgusInputLog);
	if (convexPolygon.Num() < 4)
	{
		// TODO JAMES: Error here
		return;
	}

	const FVector2D panUpDirection = FVector2D(AArgusCameraActor::GetPanUpVector());
	const FVector2D panRightDirection = FVector2D(AArgusCameraActor::GetPanRightVector());
	const FVector2D cameraLocation = FVector2D(argusCamera->GetCameraLocationWithoutZoom());
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
}

void UArgusInputManager::ProcessMoveToInputEvent(bool onAttackMove)
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

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

	EMovementState inputMovementState = EMovementState::ProcessMoveToLocationCommand;
	ArgusEntity targetEntity = ArgusEntity::k_emptyEntity;
	ArgusEntity decalEntity = ArgusEntity::k_emptyEntity;
	if (AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor()))
	{
		targetEntity = argusActor->GetEntity();
		if (targetEntity && targetEntity.GetComponent<TransformComponent>())
		{
			inputMovementState = EMovementState::ProcessMoveToEntityCommand;
		}
	}
	else if (inputInterfaceComponent->m_selectedArgusEntityIds.Num() > 0)
	{
		decalEntity = DecalSystems::InstantiateMoveToLocationDecalEntity(m_owningPlayerController->GetMoveToLocationDecalActorRecord(), targetLocation, inputInterfaceComponent->m_selectedArgusEntityIds.Num(), ArgusECSConstants::k_maxEntities, EDecalTypePolicy::DeferredPopulation);
	}

	InputInterfaceSystems::MoveSelectedEntitiesToTarget(inputMovementState, targetEntity, targetLocation, decalEntity, onAttackMove);
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

	const uint16 numWaypointEligibleEntities = InputInterfaceSystems::GetNumWaypointEligibleEntities();
	if (numWaypointEligibleEntities == 0u)
	{
		return;
	}
	
	ArgusEntity decalEntity = DecalSystems::InstantiateMoveToLocationDecalEntity(m_owningPlayerController->GetMoveToLocationDecalActorRecord(), targetLocation, numWaypointEligibleEntities, DecalSystems::GetMostRecentSelectedWaypointDecalEntityId(), EDecalTypePolicy::PopulateMoveToLocation);
	InputInterfaceSystems::SetWaypointForSelectedEntities(targetLocation, decalEntity);
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

	ARGUS_RETURN_ON_NULL(argusCamera, ArgusInputLog);

	argusCamera->UpdateCameraZoom(zoomValue);
}

void UArgusInputManager::ProcessAbilityInputEvent(EAbilityIndex abilityIndex)
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

	InputInterfaceSystems::SetAbilityStateForCastIndex(abilityIndex);
}

void UArgusInputManager::ProcessEscapeInputEvent()
{

}

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

	ARGUS_RETURN_ON_NULL(argusCamera, ArgusInputLog);

	argusCamera->UpdateCameraOrbit(rotationValue);
}

void UArgusInputManager::ProcessControlGroup(uint8 controlGroupIndex, AArgusCameraActor* argusCamera)
{
	ARGUS_RETURN_ON_NULL(argusCamera, ArgusInputLog);

	InputInterfaceSystems::SelectControlGroup(controlGroupIndex, m_owningPlayerController->GetMoveToLocationDecalActorRecord());
	argusCamera->FocusOnArgusEntity(InputInterfaceSystems::GetASelectedEntity());
}

void UArgusInputManager::ProcessSetControlGroup(uint8 controlGroupIndex)
{
	InputInterfaceSystems::SetControlGroup(controlGroupIndex);
}

void UArgusInputManager::ProcessChangeActiveAbilityGroup()
{
	InputInterfaceSystems::ChangeActiveAbilityGroup();
}

void UArgusInputManager::ProcessUserInterfaceEntityClicked(ArgusEntity entity)
{
	ARGUS_RETURN_ON_NULL(entity, ArgusInputLog);
	if (!ValidateOwningPlayerController())
	{
		return;
	}

	InputInterfaceSystems::AddSelectedEntityExclusive(entity, m_owningPlayerController->GetMoveToLocationDecalActorRecord());
}

#pragma endregion

void UArgusInputManager::SetReticleState()
{
	ARGUS_TRACE(UArgusInputManager::SetReticleState);

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	ReticleComponent* reticleComponent = singletonEntity.GetComponent<ReticleComponent>();
	ARGUS_RETURN_ON_NULL(reticleComponent, ArgusInputLog);

	if (!reticleComponent->IsReticleEnabled())
	{
		return;
	}

	const UAbilityRecord* abilityRecord = ArgusStaticData::GetRecord<UAbilityRecord>(reticleComponent->m_abilityRecordId);
	ARGUS_RETURN_ON_NULL(abilityRecord, ArgusInputLog);

	if (reticleComponent->m_wasAbilityCast)
	{
		if (abilityRecord->GetDisableReticleAfterCast())
		{
			reticleComponent->DisableReticle();
			return;
		}

		reticleComponent->m_wasAbilityCast = false;
	}

	FHitResult hitResult;
	if (!m_owningPlayerController->GetMouseProjectionLocation(ECC_RETICLE, hitResult))
	{
		return;
	}

	reticleComponent->m_reticleLocation = hitResult.Location;
	reticleComponent->m_isBlocked = SpatialPartitioningSystems::AnyObstaclesOrStaticEntitiesInCircle(reticleComponent->m_reticleLocation, reticleComponent->m_radius, AbilitySystems::GetResourceBufferRadiusOfConstructionAbility(abilityRecord));
}

void UArgusInputManager::ProcessReticleAbilityForSelectedEntities(const ReticleComponent* reticleComponent)
{
	ARGUS_RETURN_ON_NULL(reticleComponent, ArgusInputLog);

	if (CVarEnableVerboseArgusInputLogging.GetValueOnGameThread())
	{
		ARGUS_LOG
		(
			ArgusInputLog, Display, TEXT("[%s] Pressed select for Reticle Ability %d."),
			ARGUS_FUNCNAME,
			reticleComponent->m_abilityRecordId
		);
	}

	InputInterfaceSystems::SetAbilityStateForReticleAbility(reticleComponent);
}
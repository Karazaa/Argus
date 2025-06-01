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
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/TransformSystems.h"

#define ECC_RETICLE	ECC_GameTraceChannel1

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

void UArgusInputManager::OnControlGroup0(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup0, value));
}

void UArgusInputManager::OnControlGroup1(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup1, value));
}

void UArgusInputManager::OnControlGroup2(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup2, value));
}

void UArgusInputManager::OnControlGroup3(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup3, value));
}

void UArgusInputManager::OnControlGroup4(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup4, value));
}

void UArgusInputManager::OnControlGroup5(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::ControlGroup5, value));
}

void UArgusInputManager::OnSetControlGroup0(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup0, value));
}

void UArgusInputManager::OnSetControlGroup1(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup1, value));
}

void UArgusInputManager::OnSetControlGroup2(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup2, value));
}

void UArgusInputManager::OnSetControlGroup3(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup3, value));
}

void UArgusInputManager::OnSetControlGroup4(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup4, value));
}

void UArgusInputManager::OnSetControlGroup5(const FInputActionValue& value)
{
	ARGUS_MEMORY_TRACE(ArgusInputManager);
	m_inputEventsThisFrame.Emplace(InputCache(InputType::SetControlGroup5, value));
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

	if (CleanUpSelectedActors())
	{
		OnSelectedArgusArgusActorsChanged();
	}
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
			InterruptReticleFromInputEvent();
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
		case InputType::ChangeActiveAbilityGroup:
			InterruptReticleFromInputEvent();
			ProcessChangeActiveAbilityGroup();
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
			ProcessReticleAbilityForSelectedEntities(reticleComponent);
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

	bool shouldIgnoreTeamRequirement = false;

#if !UE_BUILD_SHIPPING
	shouldIgnoreTeamRequirement = ArgusECSDebugger::ShouldIgnoreTeamRequirementsForSelectingEntities();
#endif //!UE_BUILD_SHIPPING

	if (!shouldIgnoreTeamRequirement)
	{
		m_owningPlayerController->FilterArgusActorsToPlayerTeam(actorsWithinBounds);
	}
	
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

	EMovementState inputMovementState = EMovementState::ProcessMoveToLocationCommand;
	ArgusEntity targetEntity = ArgusEntity::k_emptyEntity;
	if (AArgusActor* argusActor = Cast<AArgusActor>(hitResult.GetActor()))
	{
		targetEntity = argusActor->GetEntity();
		if (targetEntity && targetEntity.GetComponent<TransformComponent>())
		{
			inputMovementState = EMovementState::ProcessMoveToEntityCommand;
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

void UArgusInputManager::ProcessMoveToInputEventPerSelectedActor(AArgusActor* argusActor, EMovementState inputMovementState, ArgusEntity targetEntity, FVector targetLocation)
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
	
	if (inputMovementState == EMovementState::ProcessMoveToEntityCommand)
	{
		if (targetEntity == selectedEntity)
		{
			taskComponent->m_movementState = EMovementState::None;
		}
		else
		{
			if (navigationComponent)
			{
				AvoidanceSystems::DecrementIdleEntitiesInGroup(selectedEntity);
				taskComponent->m_movementState = inputMovementState;
			}
			
			targetingComponent->m_targetEntityId = targetEntity.GetId();
			targetingComponent->m_targetLocation.Reset();
		}
	}
	else if (inputMovementState == EMovementState::ProcessMoveToLocationCommand)
	{
		if (navigationComponent)
		{
			AvoidanceSystems::DecrementIdleEntitiesInGroup(selectedEntity);
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
		case EMovementState::None:
		case EMovementState::FailedToFindPath:
		case EMovementState::ProcessMoveToEntityCommand:
		case EMovementState::MoveToEntity:
			taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
			targetingComponent->m_targetEntityId = ArgusEntity::k_emptyEntity.GetId();
			targetingComponent->m_targetLocation = targetLocation;
			navigationComponent->ResetQueuedWaypoints();
			break;
		case EMovementState::ProcessMoveToLocationCommand:
		case EMovementState::MoveToLocation:
			taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
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

	ARGUS_RETURN_ON_NULL(argusCamera, ArgusInputLog);

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

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return;
	}

	for (int32 i = 0; i < inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Num(); ++i)
	{
		if (inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds[i] == ArgusECSConstants::k_maxEntities)
		{
			continue;
		}

		ProcessAbilityInputEventPerSelectedEntity(ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds[i]), abilityIndex);
	}
}

void UArgusInputManager::ProcessAbilityInputEventPerSelectedEntity(const ArgusEntity& entity, uint8 abilityIndex)
{
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
			taskComponent->m_abilityState = EAbilityState::ProcessCastAbility0Command;
			break;
		case 1u:
			taskComponent->m_abilityState = EAbilityState::ProcessCastAbility1Command;
			break;
		case 2u:
			taskComponent->m_abilityState = EAbilityState::ProcessCastAbility2Command;
			break;
		case 3u:
			taskComponent->m_abilityState = EAbilityState::ProcessCastAbility3Command;
			break;
		default:
			break;
	}
}

void UArgusInputManager::ProcessEscapeInputEvent()
{

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

	ARGUS_RETURN_ON_NULL(argusCamera, ArgusInputLog);

	argusCamera->UpdateCameraOrbit(rotationValue);
}

void UArgusInputManager::ProcessControlGroup(uint8 controlGroupIndex, AArgusCameraActor* argusCamera)
{
	if (m_controlGroupActors[controlGroupIndex].IsEmpty())
	{
		return;
	}

	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		selectedActor->SetSelectionState(false);
	}
	m_selectedArgusActors = m_controlGroupActors[controlGroupIndex];
	CleanUpSelectedActors();
	OnSelectedArgusArgusActorsChanged();

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	const InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return;
	}

	if (inputInterfaceComponent->m_indexOfActiveAbilityGroup < 0 && inputInterfaceComponent->m_indexOfActiveAbilityGroup >= inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Num())
	{
		return;
	}

	ArgusEntity templateEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_indexOfActiveAbilityGroup);
	if (!templateEntity)
	{
		return;
	}

	argusCamera->FocusOnArgusEntity(templateEntity);
}

void UArgusInputManager::ProcessSetControlGroup(uint8 controlGroupIndex)
{
	m_controlGroupActors[controlGroupIndex] = m_selectedArgusActors;
}

void UArgusInputManager::ProcessChangeActiveAbilityGroup()
{
	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		// TODO JAMES: Error here
		return;
	}

	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		// TODO JAMES: Error here
		return;
	}

	if (inputInterfaceComponent->m_selectedArgusEntityIds.Num() == 0)
	{
		return;
	}

	const int8 previousIndexOfActiveAbilityGroup = inputInterfaceComponent->m_indexOfActiveAbilityGroup;
	if (previousIndexOfActiveAbilityGroup < 0)
	{
		return;
	}

	const AbilityComponent* previousActiveAbilityGroupAbilities = nullptr;
	if (ArgusEntity previousTemplateEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[previousIndexOfActiveAbilityGroup]))
	{
		previousActiveAbilityGroupAbilities = previousTemplateEntity.GetComponent<AbilityComponent>();
	}

	const AbilityComponent* templateEntityAbilities = nullptr;
	for (int8 i = 1; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		int8 indexToCheck = ((previousIndexOfActiveAbilityGroup + i) % inputInterfaceComponent->m_selectedArgusEntityIds.Num());

		ArgusEntity entityToCheck = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[indexToCheck]);
		if (!entityToCheck)
		{
			continue;
		}

		const AbilityComponent* abilityComponentToCheck = entityToCheck.GetComponent<AbilityComponent>();
		if (!abilityComponentToCheck)
		{
			continue;
		}

		if (previousActiveAbilityGroupAbilities && previousActiveAbilityGroupAbilities->HasSameAbilities(abilityComponentToCheck))
		{
			continue;
		}

		if (templateEntityAbilities == nullptr)
		{
			templateEntityAbilities = abilityComponentToCheck;
			inputInterfaceComponent->m_indexOfActiveAbilityGroup = indexToCheck;
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Empty();
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(entityToCheck.GetId());
			inputInterfaceComponent->m_selectedActorsDisplayState = ESelectedActorsDisplayState::ChangedThisFrame;
			continue;
		}

		if (templateEntityAbilities->HasSameAbilities(abilityComponentToCheck))
		{
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(entityToCheck.GetId());
		}
	}
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

	const int32 selectedActorsNum = marqueeSelectedActors.Num();
	const int32 existingSelectedActorNum = m_selectedArgusActors.Num();
	m_selectedArgusActors.Reserve(selectedActorsNum + existingSelectedActorNum);

	for (int32 i = 0; i < selectedActorsNum; ++i)
	{
		if (marqueeSelectedActors[i])
		{
			marqueeSelectedActors[i]->SetSelectionState(true);
			m_selectedArgusActors.Emplace(marqueeSelectedActors[i]);
		}
	}

	if (selectedActorsNum > 0)
	{
		OnSelectedArgusArgusActorsChanged();
	}
}

bool UArgusInputManager::CleanUpSelectedActors()
{
	bool removedActors = false;

	TArray<TWeakObjectPtr<AArgusActor>> noLongerSelectableArgusActors;
	for (TWeakObjectPtr<AArgusActor>& selectedActor : m_selectedArgusActors)
	{
		if (!selectedActor.IsValid())
		{
			continue;
		}

		if (selectedActor->GetEntity().IsAlive() && !selectedActor->GetEntity().IsPassenger())
		{
			selectedActor->SetSelectionState(true);
			continue;
		}

		selectedActor->SetSelectionState(false);
		noLongerSelectableArgusActors.Add(selectedActor);
	}

	for (TWeakObjectPtr<AArgusActor>& noLongerSelectableArgusActor : noLongerSelectableArgusActors)
	{
		removedActors = true;
		m_selectedArgusActors.Remove(noLongerSelectableArgusActor);
	}

	return removedActors;
}

void UArgusInputManager::OnSelectedArgusArgusActorsChanged()
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusInputLog);

	inputInterfaceComponent->m_selectedArgusEntityIds.Reset();
	inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Reset();
	inputInterfaceComponent->m_indexOfActiveAbilityGroup = -1;
	inputInterfaceComponent->m_selectedActorsDisplayState = ESelectedActorsDisplayState::ChangedThisFrame;

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
	}

	inputInterfaceComponent->m_selectedArgusEntityIds.Sort([](uint16 entityAId, uint16 entityBId)
	{
		const AbilityComponent* taskComponentA = ArgusEntity::RetrieveEntity(entityAId).GetComponent<AbilityComponent>();
		const AbilityComponent* taskComponentB = ArgusEntity::RetrieveEntity(entityBId).GetComponent<AbilityComponent>();
		if (!taskComponentA)
		{
			return false;
		}
		if (!taskComponentB)
		{
			return true;
		}

		return taskComponentA->m_abilityCasterPriority > taskComponentB->m_abilityCasterPriority;
	});

	const int32 numSelected = inputInterfaceComponent->m_selectedArgusEntityIds.Num();
	if (numSelected == 0)
	{
		return;
	}

	const AbilityComponent* templateAbilityComponent = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[0]).GetComponent<AbilityComponent>();
	if (!templateAbilityComponent || !templateAbilityComponent->HasAnyAbility())
	{
		return;
	}

	inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(inputInterfaceComponent->m_selectedArgusEntityIds[0]);
	for (int32 i = 1; i < numSelected; ++i)
	{
		const AbilityComponent* abilityComponent = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]).GetComponent<AbilityComponent>();
		if (!abilityComponent || !abilityComponent->HasAnyAbility() || !abilityComponent->HasSameAbilities(templateAbilityComponent))
		{
			break;
		}

		inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
	}

	inputInterfaceComponent->m_indexOfActiveAbilityGroup = 0;
}

void UArgusInputManager::InterruptReticleFromInputEvent()
{
	ReticleComponent* reticleComponent = ArgusEntity::GetSingletonEntity().GetComponent<ReticleComponent>();
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

void UArgusInputManager::ProcessReticleAbilityForSelectedEntities(const ReticleComponent* reticleComponent)
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

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	if (!singletonEntity)
	{
		return;
	}

	const InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	if (!inputInterfaceComponent)
	{
		return;
	}

	for (int32 i = 0; i < inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Num(); ++i)
	{
		if (inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds[i] == ArgusECSConstants::k_maxEntities)
		{
			continue;
		}

		ProcessReticleAbilityPerSelectedEntity(ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds[i]), reticleComponent->m_abilityRecordId);
	}
}

void UArgusInputManager::ProcessReticleAbilityPerSelectedEntity(const ArgusEntity& entity, uint32 abilityRecordId)
{
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

	taskComponent->m_abilityState = EAbilityState::ProcessCastReticleAbility;
}

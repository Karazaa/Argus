// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "ArgusCameraActor.h"
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "UObject/SoftObjectPtr.h"
#include "ArgusInputManager.generated.h"

class	AArgusPlayerController;
class	UArgusInputActionSet;
class	UEnhancedInputComponent;
class   UEnhancedPlayerInput;
struct	FInputActionValue;
struct	ReticleComponent;
struct  TargetingComponent;

UCLASS()
class UArgusInputManager : public UObject
{
	GENERATED_BODY()

public:
	enum class InputType : uint8
	{
		None,
		Select,
		SelectAdditive,
		MarqueeSelect,
		MarqueeSelectAdditive,
		MoveTo,
		SetWaypoint,
		Zoom,
		Ability0,
		Ability1,
		Ability2,
		Ability3,
		Escape,
		RotateCamera,
		StartPanningLockout,
		StopPanningLockout,
		ControlGroup0,
		ControlGroup1,
		ControlGroup2,
		ControlGroup3,
		ControlGroup4,
		ControlGroup5,
		SetControlGroup0,
		SetControlGroup1,
		SetControlGroup2,
		SetControlGroup3,
		SetControlGroup4,
		SetControlGroup5,
		ChangeActiveAbilityGroup,
		UserInterfaceEntityClicked,
	};
	static bool ShouldUpdateSelectedActorDisplay(ArgusEntity& templateSelectedEntity);

	void SetupInputComponent(AArgusPlayerController* owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet);
	void OnSelect(const FInputActionValue& value);
	void OnSelectAdditive(const FInputActionValue& value);
	void OnMarqueeSelect(const FInputActionValue& value);
	void OnMarqueeSelectAdditive(const FInputActionValue& value);
	void OnMoveTo(const FInputActionValue& value);
	void OnSetWaypoint(const FInputActionValue& value);
	void OnZoom(const FInputActionValue& value);
	void OnAbility0(const FInputActionValue& value);
	void OnAbility1(const FInputActionValue& value);
	void OnAbility2(const FInputActionValue& value);
	void OnAbility3(const FInputActionValue& value);
	void OnEscape(const FInputActionValue& value);
	void OnRotateCamera(const FInputActionValue& value);
	void OnStartPanningLockout(const FInputActionValue& value);
	void OnStopPanningLockout(const FInputActionValue& value);
	void OnUserInterfaceButtonClicked(InputType inputEvent);
	void OnUserInterfaceEntityClicked(const ArgusEntity& clickedEntity);
	void OnControlGroup0(const FInputActionValue& value);
	void OnControlGroup1(const FInputActionValue& value);
	void OnControlGroup2(const FInputActionValue& value);
	void OnControlGroup3(const FInputActionValue& value);
	void OnControlGroup4(const FInputActionValue& value);
	void OnControlGroup5(const FInputActionValue& value);
	void OnSetControlGroup0(const FInputActionValue& value);
	void OnSetControlGroup1(const FInputActionValue& value);
	void OnSetControlGroup2(const FInputActionValue& value);
	void OnSetControlGroup3(const FInputActionValue& value);
	void OnSetControlGroup4(const FInputActionValue& value);
	void OnSetControlGroup5(const FInputActionValue& value);
	void OnChangeActiveAbilityGroup(const FInputActionValue& value);

	void ProcessPlayerInput(AArgusCameraActor* argusCamera, const AArgusCameraActor::UpdateCameraPanningParameters& updateCameraPanningParameters, float deltaTime);
	bool ShouldDrawMarqueeBox() const;

	const FVector& GetSelectionStartWorldSpaceLocation() const;
	const FVector2D GetSelectionStartScreenSpaceLocation() const;

private:
	struct InputCache
	{
		const InputType m_type;
		const ArgusEntity m_entity;
		const FInputActionValue m_value;

		InputCache(const InputType type, const FInputActionValue value) : m_type(type), m_entity(ArgusEntity::k_emptyEntity), m_value(value) {}
		InputCache(const InputType type, const ArgusEntity& entity) : m_type(type), m_entity(entity), m_value(FInputActionValue()) {}
	};

	TWeakObjectPtr<AArgusPlayerController> m_owningPlayerController = nullptr;
	TSet<TWeakObjectPtr<AArgusActor>> m_controlGroupActors[6];
	TArray<InputCache> m_inputEventsThisFrame;

	void BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, UEnhancedInputComponent* enhancedInputComponent, UEnhancedPlayerInput* enhancedInput);
	bool ValidateOwningPlayerController();

	void PrepareToProcessInputEvents();
	void ProcessInputEvent(AArgusCameraActor* argusCamera, const InputCache& inputEvent);
	void ProcessSelectInputEvent(bool isAdditive);
	void ProcessMarqueeSelectInputEvent(const AArgusCameraActor* argusCamera, bool isAdditive);
	void PopulateMarqueeSelectPolygon(const AArgusCameraActor* argusCamera, TArray<FVector2D>& convexPolygon);
	void ProcessMoveToInputEvent();
	void ProcessSetWaypointInputEvent();
	void ProcessZoomInputEvent(AArgusCameraActor* argusCamera, const FInputActionValue& value);
	void ProcessAbilityInputEvent(uint8 abilityIndex);
	void ProcessAbilityInputEventPerSelectedEntity(const ArgusEntity& entity, uint8 abilityIndex);
	void ProcessEscapeInputEvent();
	void ProcessRotateCameraInputEvent(AArgusCameraActor* argusCamera, const FInputActionValue& value);
	void ProcessControlGroup(uint8 controlGroupIndex, AArgusCameraActor* argusCamera);
	void ProcessSetControlGroup(uint8 controlGroupIndex);
	void ProcessChangeActiveAbilityGroup();
	void ProcessUserInterfaceEntityClicked(const ArgusEntity& entity);

	void SetReticleState();
	void ProcessReticleAbilityForSelectedEntities(const ReticleComponent* reticleComponent);
	void ProcessReticleAbilityPerSelectedEntity(const ArgusEntity& entity, uint32 abilityRecordId);

	FVector m_cachedLastSelectInputWorldSpaceLocation = FVector::ZeroVector;
	bool m_selectInputDown = false;
	bool m_canRotateCamera = false;
};

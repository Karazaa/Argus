// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "ArgusCameraActor.h"
#include "ArgusUIButtonClickedEventsEnum.h"
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

UCLASS()
class UArgusInputManager : public UObject
{
	GENERATED_BODY()

public:
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
	void OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent);
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


	void ProcessPlayerInput(AArgusCameraActor* argusCamera, const AArgusCameraActor::UpdateCameraPanningParameters& updateCameraPanningParameters, float deltaTime);
	bool ShouldUpdateSelectedActorDisplay(ArgusEntity& templateSelectedEntity) const;
	bool ShouldDrawMarqueeBox() const;

	const FVector& GetSelectionStartWorldSpaceLocation() const;
	const FVector2D GetSelectionStartScreenSpaceLocation() const;

private:
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
		SetControlGroup5
	};

	struct InputCache
	{
		const InputType m_type;
		const FInputActionValue m_value;

		InputCache(const InputType type, const FInputActionValue value) : m_type(type), m_value(value) { }
	};

	TWeakObjectPtr<AArgusPlayerController> m_owningPlayerController = nullptr;
	TSet<TWeakObjectPtr<AArgusActor>> m_selectedArgusActors;
	TSet<TWeakObjectPtr<AArgusActor>> m_controlGroupActors[6];
	TSet<TWeakObjectPtr<AArgusActor>> m_activeAbilityGroupArgusActors;
	TArray<InputCache> m_inputEventsThisFrame;

	void BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, UEnhancedInputComponent* enhancedInputComponent, UEnhancedPlayerInput* enhancedInput);
	bool ValidateOwningPlayerController();

	void PrepareToProcessInputEvents();
	void ProcessInputEvent(AArgusCameraActor* argusCamera, const InputCache& inputEvent);
	void ProcessSelectInputEvent(bool isAdditive);
	void ProcessMarqueeSelectInputEvent(AArgusCameraActor* argusCamera, bool isAdditive);
	void ProcessMoveToInputEvent();
	void ProcessMoveToInputEventPerSelectedActor(AArgusActor* argusActor, EMovementState inputMovementState, ArgusEntity targetEntity, FVector targetLocation);
	void ProcessSetWaypointInputEvent();
	void ProcessSetWaypointInputEventPerSelectedActor(AArgusActor* argusActor, FVector targetLocation);
	void ProcessZoomInputEvent(AArgusCameraActor* argusCamera, const FInputActionValue& value);
	void ProcessAbilityInputEvent(uint8 abilityIndex);
	void ProcessAbilityInputEventPerSelectedActor(AArgusActor* argusActor, uint8 abilityIndex);
	void ProcessEscapeInputEvent();
	void ProcessRotateCameraInputEvent(AArgusCameraActor* argusCamera, const FInputActionValue& value);
	void ProcessControlGroup(uint8 controlGroupIndex, AArgusCameraActor* argusCamera);
	void ProcessSetControlGroup(uint8 controlGroupIndex);

	void AddSelectedActorExclusive(AArgusActor* argusActor);
	void AddSelectedActorAdditive(AArgusActor* argusActor);
	void AddMarqueeSelectedActorsExclusive(const TArray<AArgusActor*>& marqueeSelectedActors);
	void AddMarqueeSelectedActorsAdditive(const TArray<AArgusActor*>& marqueeSelectedActors);
	bool CleanUpSelectedActors();
	void OnSelectedArgusArgusActorsChanged();

	void InterruptReticleFromInputEvent();
	void SetReticleState();
	void ProcessReticleAbilityForSelectedActors(const ReticleComponent* reticleComponent);
	void ProcessReticleAbilityPerSelectedActor(AArgusActor* argusActor, uint32 abilityRecordId);

	FVector m_cachedLastSelectInputWorldSpaceLocation = FVector::ZeroVector;
	bool m_selectedArgusActorsChangedThisFrame = false;
	bool m_selectInputDown = false;
	bool m_canRotateCamera = false;
};

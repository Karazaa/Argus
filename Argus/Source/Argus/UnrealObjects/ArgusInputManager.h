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
struct	FInputActionValue;
struct	ReticleComponent;

UCLASS()
class UArgusInputManager : public UObject
{
	GENERATED_BODY()

public:
	void SetupInputComponent(TWeakObjectPtr<AArgusPlayerController> owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet);
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
	void OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent);


	void ProcessPlayerInput(TObjectPtr<AArgusCameraActor>& argusCamera, const AArgusCameraActor::UpdateCameraPanningParameters& updateCameraPanningParameters, float deltaTime);
	bool ShouldUpdateSelectedActorDisplay(ArgusEntity& templateSelectedEntity);

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
		Escape
	};

	struct InputCache
	{
		const InputType m_type;
		const FInputActionValue m_value;

		InputCache(const InputType type, const FInputActionValue value) : m_type(type), m_value(value) { }
	};

	TWeakObjectPtr<AArgusPlayerController> m_owningPlayerController = nullptr;
	TSet<TWeakObjectPtr<AArgusActor>> m_selectedArgusActors;
	TSet<TWeakObjectPtr<AArgusActor>> m_activeAbilityGroupArgusActors;
	TArray<InputCache> m_inputEventsThisFrame;

	void BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, TWeakObjectPtr<UEnhancedInputComponent>& enhancedInputComponent);
	bool ValidateOwningPlayerController();

	void PrepareToProcessInputEvents();
	void ProcessInputEvent(TObjectPtr<AArgusCameraActor>& argusCamera, const InputCache& inputEvent);
	void ProcessSelectInputEvent(bool isAdditive);
	void ProcessMarqueeSelectInputEvent(bool isAdditive);
	void ProcessMoveToInputEvent();
	void ProcessMoveToInputEventPerSelectedActor(AArgusActor* argusActor, MovementState inputMovementState, ArgusEntity targetEntity, FVector targetLocation);
	void ProcessSetWaypointInputEvent();
	void ProcessSetWaypointInputEventPerSelectedActor(AArgusActor* argusActor, FVector targetLocation);
	void ProcessZoomInputEvent(TObjectPtr<AArgusCameraActor>& argusCamera, const FInputActionValue& value);
	void ProcessAbilityInputEvent(uint8 abilityIndex);
	void ProcessAbilityInputEventPerSelectedActor(AArgusActor* argusActor, uint8 abilityIndex);
	void ProcessEscapeInputEvent();

	void AddSelectedActorExclusive(AArgusActor* argusActor);
	void AddSelectedActorAdditive(AArgusActor* argusActor);
	void AddMarqueeSelectedActorsExclusive(const TArray<AArgusActor*>& marqueeSelectedActors);
	void AddMarqueeSelectedActorsAdditive(const TArray<AArgusActor*>& marqueeSelectedActors);
	void OnSelectedArgusArgusActorsChanged();

	void InterruptReticleFromInputEvent();
	void SetReticleState();
	void ProcessReticleAbilityForSelectedActors(const ReticleComponent* reticleComponent);
	void ProcessReticleAbilityPerSelectedActor(AArgusActor* argusActor, uint32 abilityRecordId);

	FVector m_cachedLastSelectInputWorldspaceLocation = FVector::ZeroVector;
	bool m_selectedArgusActorsChangedThisFrame = false;
};

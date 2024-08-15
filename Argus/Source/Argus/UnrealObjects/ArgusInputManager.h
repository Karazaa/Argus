// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "UObject/SoftObjectPtr.h"
#include "ArgusInputManager.generated.h"

struct FInputActionValue;
class AArgusPlayerController;
class UArgusInputActionSet;
class UEnhancedInputComponent;

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

	void ProcessPlayerInput();

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
		Zoom
	};

	struct InputCache
	{
		const InputType m_type;
		const FInputActionValue m_value;

		InputCache(const InputType type, const FInputActionValue value) : m_type(type), m_value(value) { }
	};

	TWeakObjectPtr<AArgusPlayerController> m_owningPlayerController = nullptr;
	TSet<TWeakObjectPtr<AArgusActor>> m_selectedArgusActors;
	TArray<InputCache> m_inputEventsThisFrame;

	void BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, TWeakObjectPtr<UEnhancedInputComponent>& enhancedInputComponent);
	bool ValidateOwningPlayerController();

	void ProcessInputEvent(const InputCache& inputEvent);
	void ProcessSelectInputEvent(bool isAdditive);
	void ProcessMarqueeSelectInputEvent(bool isAdditive);
	void ProcessMoveToInputEvent();
	void ProcessMoveToInputEventPerSelectedActor(AArgusActor* argusActor, ETask inputTask, ArgusEntity targetEntity, FVector targetLocation);
	void ProcessSetWaypointInputEvent();
	void ProcessSetWaypointInputEventPerSelectedActor(AArgusActor* argusActor, FVector targetLocation);
	void ProcessZoomInpuEvent(const FInputActionValue& value);

	void AddSelectedActorExclusive(AArgusActor* argusActor);
	void AddSelectedActorAdditive(AArgusActor* argusActor);
	void AddMarqueeSelectedActorsExclusive(const TArray<AArgusActor*>& marqueeSelectedActors);
	void AddMarqueeSelectedActorsAdditive(const TArray<AArgusActor*>& marqueeSelectedActors);

	FVector m_cachedLastSelectInputWorldspaceLocation = FVector::ZeroVector;
};

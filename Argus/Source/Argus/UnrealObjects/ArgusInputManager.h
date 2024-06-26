// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusActor.h"
#include "CoreMinimal.h"
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
	void OnMoveTo(const FInputActionValue& value);

	void ProcessPlayerInput();

private:
	enum class InputType : uint8
	{
		None,
		Select,
		SelectAdditive,
		MoveTo
	};

	TWeakObjectPtr<AArgusPlayerController> m_owningPlayerController = nullptr;
	TSet<TWeakObjectPtr<AArgusActor>> m_selectedArgusActors;
	TArray<InputType> m_inputEventsThisFrame;

	void BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, TWeakObjectPtr<UEnhancedInputComponent>& enhancedInputComponent);
	bool ValidateOwningPlayerController();

	void ProcessInputEvent(InputType inputEvent);
	void ProcessSelectInputEvent(bool isAdditive);
	void ProcessMoveToInputEvent();
	void ProcessMoveToInputEventPerSelectedActor(AArgusActor* argusActor, ETask inputTask, ArgusEntity targetEntity, FVector targetLocation);
	void AddSelectedActorExclusive(AArgusActor* argusActor);
	void AddSelectedActorAdditive(AArgusActor* argusActor);
};

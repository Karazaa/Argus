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
	void ProcessPlayerInput();
	void SetupInputComponent(TWeakObjectPtr<AArgusPlayerController> owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet);
	void OnSelect(const FInputActionValue& value);
	void OnSelectAdditive(const FInputActionValue& value);
	void OnMoveTo(const FInputActionValue& value);

private:
	TWeakObjectPtr<AArgusPlayerController> m_owningPlayerController = nullptr;
	TSet<TWeakObjectPtr<AArgusActor>> m_selectedArgusActors;

	void BindActions(TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet, TWeakObjectPtr<UEnhancedInputComponent>& enhancedInputComponent);
	void OnSelectInternal(bool isAdditive);

	bool ValidateOwningPlayerController();
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "ArgusInputManager.generated.h"

struct FInputActionValue;
class UArgusInputActionSet;
class UInputComponent;

UCLASS()
class UArgusInputManager : public UObject
{
	GENERATED_BODY()
public:
	void SetupInputComponent(TObjectPtr<UInputComponent>& inputComponent, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet);
	void OnSelect(const FInputActionValue& value);
	void OnMoveTo(const FInputActionValue& value);
};

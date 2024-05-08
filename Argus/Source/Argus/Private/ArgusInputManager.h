// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "ArgusInputManager.generated.h"

struct FInputActionValue;
class UArgusInputActionSet;
class AArgusPlayerController;

UCLASS()
class UArgusInputManager : public UObject
{
	GENERATED_BODY()
public:
	void ProcessPlayerInput();
	void SetupInputComponent(TWeakObjectPtr<AArgusPlayerController> owningPlayerController, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet);
	void OnSelect(const FInputActionValue& value);
	void OnMoveTo(const FInputActionValue& value);

private:
	TWeakObjectPtr<AArgusPlayerController> m_owningPlayerController = nullptr;
	TArray<ArgusEntity> m_selectedEntities;
};

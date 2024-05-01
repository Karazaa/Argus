// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"

class UArgusInputActionSet;
class UInputComponent;

class ArgusInputManager
{
public:
	void SetupInputComponent(TObjectPtr<UInputComponent>& inputComponent, TSoftObjectPtr<UArgusInputActionSet>& argusInputActionSet);
};

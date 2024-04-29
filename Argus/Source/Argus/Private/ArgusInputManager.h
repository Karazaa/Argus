// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusInputManager.generated.h"

class UInputComponent;

UCLASS()
class ARGUS_API UArgusInputManager : public UDataAsset
{
	GENERATED_BODY()

public:
	void SetupInputComponent(TObjectPtr<UInputComponent>& inputComponent);
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusInputActionSet.h"
#include "ArgusInputManager.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "ArgusPlayerController.generated.h"

UCLASS()
class AArgusPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UInputMappingContext> m_argusInputMappingContext = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UArgusInputActionSet> m_argusInputActionSet = nullptr;

	TObjectPtr<UArgusInputManager> m_argusInputManager = nullptr;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};

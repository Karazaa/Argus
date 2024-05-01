// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusInputActionSet.h"
#include "ArgusInputManager.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArgusPlayerController.generated.h"

UCLASS()
class AArgusPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSoftObjectPtr<UArgusInputActionSet> m_argusInputActionSet = nullptr;

	ArgusInputManager m_argusInputManager = ArgusInputManager();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};

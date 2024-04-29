// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusInputManager.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArgusPlayerController.generated.h"

UCLASS()
class AArgusPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSoftObjectPtr<UArgusInputManager> m_argusInputManager = nullptr;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
};

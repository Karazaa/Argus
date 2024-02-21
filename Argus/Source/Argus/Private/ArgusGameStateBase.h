// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ArgusGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class AArgusGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void HandleBeginPlay() override;
};

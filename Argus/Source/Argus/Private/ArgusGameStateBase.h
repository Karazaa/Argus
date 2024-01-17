// Fill out your copyright notice in the Description page of Project Settings.

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

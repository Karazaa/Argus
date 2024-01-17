// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArgusGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class AArgusGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AArgusGameModeBase();
	virtual void StartPlay() override;
};

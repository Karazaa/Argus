// Fill out your copyright notice in the Description page of Project Settings.

#include "ArgusGameModeBase.h"
#include "ArgusGameStateBase.h"
#include "ArgusUtil.h"

AArgusGameModeBase::AArgusGameModeBase()
{
	UE_LOG(ArgusGameLog, Display, TEXT("Constructing Argus game mode base."));
	GameStateClass = AArgusGameStateBase::StaticClass();
}

void AArgusGameModeBase::StartPlay()
{
	UE_LOG(ArgusGameLog, Display, TEXT("Argus game mode base starting play."));
}
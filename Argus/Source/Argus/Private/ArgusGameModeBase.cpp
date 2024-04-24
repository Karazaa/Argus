// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameModeBase.h"
#include "../ECS/ArgusEntity.h"
#include "ArgusGameStateBase.h"
#include "ArgusUtil.h"

AArgusGameModeBase::AArgusGameModeBase()
{
	UE_LOG(ArgusGameLog, Display, TEXT("Constructing Argus game mode base."));
	GameStateClass = AArgusGameStateBase::StaticClass();
}

void AArgusGameModeBase::StartPlay()
{
	ArgusEntity::FlushAllEntities();
	Super::StartPlay();
	UE_LOG(ArgusGameLog, Display, TEXT("Argus game mode base starting play."));
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameStateBase.h"
#include "ArgusUtil.h"

void AArgusGameStateBase::HandleBeginPlay()
{
	Super::HandleBeginPlay();
	UE_LOG(ArgusGameLog, Display, TEXT("Argus game state base handling BeginPlay."));
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusGameStateBase.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void AArgusGameStateBase::HandleBeginPlay()
{
	Super::HandleBeginPlay();
	UE_LOG(ArgusUnrealObjectsLog, Display, TEXT("Argus game state base handling BeginPlay."));
}
// Copyright Epic Games, Inc. All Rights Reserved.

#include "Argus.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(ArgusLog);
IMPLEMENT_PRIMARY_GAME_MODULE(ArgusModule, Argus, "Argus" );

void ArgusModule::StartupModule() 
{
	UE_LOG(ArgusLog, Display, TEXT("Argus module starting up."));
}

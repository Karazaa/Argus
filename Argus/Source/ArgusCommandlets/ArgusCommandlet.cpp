// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCommandlet.h"

UArgusCommandlet::UArgusCommandlet()
{
	IsClient = true;
	IsEditor = true;
	IsServer = true;
	LogToConsole = true;
}

int32 UArgusCommandlet::Main(const FString& parameters)
{
	OnStart();
	DoWork();
	OnFinish();

	return 0;
}

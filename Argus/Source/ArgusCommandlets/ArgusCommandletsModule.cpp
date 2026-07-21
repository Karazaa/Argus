// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCommandletsModule.h"

IMPLEMENT_MODULE(ArgusCommandletsModule, ArgusCommandlets);

ArgusCommandletsModule& ArgusCommandletsModule::Get()
{
	return FModuleManager::GetModuleChecked<ArgusCommandletsModule>("ArgusCommandlets");
}

void ArgusCommandletsModule::StartupModule()
{

}

void ArgusCommandletsModule::ShutdownModule()
{

}
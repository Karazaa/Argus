// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class ArgusCommandletsModule : public IModuleInterface
{
public:
	static ArgusCommandletsModule& Get();

	void StartupModule() override;
	void ShutdownModule() override;

	bool IsGameModule() const override { return false; }
};
// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(ArgusLog, Display, All);

class ArgusModule : public IModuleInterface
{
public:
	virtual bool IsGameModule() const override { return true; }
	virtual void StartupModule() override;
};
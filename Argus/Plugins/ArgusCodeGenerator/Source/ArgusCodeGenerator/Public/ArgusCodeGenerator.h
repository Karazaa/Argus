// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSComponentAdder.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class SWindow;

class FArgusCodeGeneratorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void GenerateCode();
	void AddComponent();

private:

	void RegisterMenus();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<ArgusECSComponentAdder> m_ecsTypeAdder;
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class ArgusECSObjectAdder;
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
	void OpenECSObjectAdder();

private:

	void RegisterMenus();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<ArgusECSObjectAdder> m_ecsTypeAdder;
};

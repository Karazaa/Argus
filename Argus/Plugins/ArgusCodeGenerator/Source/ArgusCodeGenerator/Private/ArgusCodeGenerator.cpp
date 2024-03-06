// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArgusCodeGenerator.h"
#include "ArgusCodeGeneratorStyle.h"
#include "ArgusCodeGeneratorCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

DEFINE_LOG_CATEGORY(ArgusCodeGeneratorLog);

static const FName ArgusCodeGeneratorTabName("ArgusCodeGenerator");

#define LOCTEXT_NAMESPACE "FArgusCodeGeneratorModule"

void FArgusCodeGeneratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FArgusCodeGeneratorStyle::Initialize();
	FArgusCodeGeneratorStyle::ReloadTextures();

	FArgusCodeGeneratorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FArgusCodeGeneratorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FArgusCodeGeneratorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FArgusCodeGeneratorModule::RegisterMenus));
}

void FArgusCodeGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FArgusCodeGeneratorStyle::Shutdown();

	FArgusCodeGeneratorCommands::Unregister();
}

void FArgusCodeGeneratorModule::PluginButtonClicked()
{
	//// Put your "OnButtonClicked" stuff here
	//FText DialogText = FText::Format(
	//						LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
	//						FText::FromString(TEXT("FArgusCodeGeneratorModule::PluginButtonClicked()")),
	//						FText::FromString(TEXT("ArgusCodeGenerator.cpp"))
	//				   );
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	UE_LOG(ArgusCodeGeneratorLog, Warning, TEXT("[%s] Pressed the plugin button!"), ARGUS_FUNCNAME);
}

void FArgusCodeGeneratorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FArgusCodeGeneratorCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FArgusCodeGeneratorCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FArgusCodeGeneratorModule, ArgusCodeGenerator)
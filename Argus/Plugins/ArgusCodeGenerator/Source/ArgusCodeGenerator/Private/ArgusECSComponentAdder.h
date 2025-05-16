// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

class ArgusECSComponentAdder
{
public:
	static TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	static void OnClicked();
	static void OnCheckBoxChecked(ECheckBoxState checkBoxState);
	static void OnTextChanged(const FText& text);
};
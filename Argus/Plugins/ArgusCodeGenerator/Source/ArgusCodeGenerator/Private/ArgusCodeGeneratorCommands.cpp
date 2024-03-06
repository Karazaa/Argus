// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArgusCodeGeneratorCommands.h"

#define LOCTEXT_NAMESPACE "FArgusCodeGeneratorModule"

void FArgusCodeGeneratorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "ArgusCodeGenerator", "Generate Argus C++ Classes", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

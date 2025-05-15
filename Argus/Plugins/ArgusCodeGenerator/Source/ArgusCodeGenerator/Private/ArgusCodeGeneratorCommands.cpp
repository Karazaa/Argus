// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCodeGeneratorCommands.h"

#define LOCTEXT_NAMESPACE "FArgusCodeGeneratorModule"

void FArgusCodeGeneratorCommands::RegisterCommands()
{
	UI_COMMAND(GenerateCode, "ArgusCodeGenerator", "Generate Argus C++ Classes", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddComponent, "ArgusCodeGenerator", "Add an ECS Component", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

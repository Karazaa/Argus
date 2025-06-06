// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ArgusCodeGeneratorStyle.h"

class FArgusCodeGeneratorCommands : public TCommands<FArgusCodeGeneratorCommands>
{
public:

	FArgusCodeGeneratorCommands()
		: TCommands<FArgusCodeGeneratorCommands>(TEXT("ArgusCodeGenerator"), NSLOCTEXT("Contexts", "ArgusCodeGenerator", "ArgusCodeGenerator Plugin"), NAME_None, FArgusCodeGeneratorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > GenerateCode;
	TSharedPtr< FUICommandInfo > AddECSObject;
};

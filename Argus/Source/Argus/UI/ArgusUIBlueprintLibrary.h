// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ArgusUIBlueprintLibrary.generated.h"

class UArgusInputManager;

UCLASS()
class UArgusUIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Could put some utility functions here. Used to have a need, but don't have much of one right now.
	// This will likely get used/expanded on at some point.

private:
	static UArgusInputManager* GetArgusInputManager(UObject* worldContextObject);
};
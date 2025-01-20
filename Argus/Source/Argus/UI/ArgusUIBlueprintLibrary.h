//Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusUIButtonClickedEventsEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ArgusUIBlueprintLibrary.generated.h"

UCLASS()
class UArgusUIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent, UObject* worldContextObject);
};
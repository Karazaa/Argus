// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusUIBlueprintLibrary.h"
#include "ArgusGameModeBase.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusPlayerController.h"
#include "Engine/World.h"

void UArgusUIBlueprintLibrary::OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent, UObject* worldContextObject)
{
	if (!worldContextObject)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Was not passed a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(worldContextObject));
		return;
	}

	const UWorld* worldPointer = worldContextObject->GetWorld();
	if (!worldPointer)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld*));
		return;
	}

	const AArgusGameModeBase* gameModePointer = worldPointer->GetAuthGameMode<AArgusGameModeBase>();
	if (!gameModePointer)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(AArgusGameModeBase*));
		return;
	}

	const AArgusPlayerController* activePlayerController = gameModePointer->GetActivePlayerController();
	if (!activePlayerController)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(AArgusPlayerController*));
		return;
	}

	UArgusInputManager* inputManager = activePlayerController->GetInputManager();
	if (!inputManager)
	{
		ARGUS_LOG(ArgusInputLog, Error, TEXT("[%s] Could not retrieve a valid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UArgusInputManager*));
		return;
	}

	inputManager->OnUserInterfaceButtonClicked(buttonClickedEvent);
}

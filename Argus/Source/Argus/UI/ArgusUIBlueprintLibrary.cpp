// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusUIBlueprintLibrary.h"
#include "ArgusGameModeBase.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusPlayerController.h"
#include "Engine/World.h"

void UArgusUIBlueprintLibrary::OnUserInterfaceButtonClicked(UArgusUIButtonClickedEventsEnum buttonClickedEvent, UObject* worldContextObject)
{
	ARGUS_RETURN_ON_NULL(worldContextObject, ArgusInputLog);

	const UWorld* worldPointer = worldContextObject->GetWorld();
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusInputLog);

	const AArgusGameModeBase* gameModePointer = worldPointer->GetAuthGameMode<AArgusGameModeBase>();
	ARGUS_RETURN_ON_NULL(gameModePointer, ArgusInputLog);

	const AArgusPlayerController* activePlayerController = gameModePointer->GetActivePlayerController();
	ARGUS_RETURN_ON_NULL(activePlayerController, ArgusInputLog);

	UArgusInputManager* inputManager = activePlayerController->GetInputManager();
	ARGUS_RETURN_ON_NULL(inputManager, ArgusInputLog);

	inputManager->OnUserInterfaceButtonClicked(buttonClickedEvent);
}

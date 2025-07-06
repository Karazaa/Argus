// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusUIBlueprintLibrary.h"
#include "ArgusGameModeBase.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusPlayerController.h"
#include "Engine/World.h"

UArgusInputManager* UArgusUIBlueprintLibrary::GetArgusInputManager(UObject* worldContextObject)
{
	ARGUS_RETURN_ON_NULL_POINTER(worldContextObject, ArgusInputLog);

	const UWorld* worldPointer = worldContextObject->GetWorld();
	ARGUS_RETURN_ON_NULL_POINTER(worldPointer, ArgusInputLog);

	const AArgusGameModeBase* gameModePointer = worldPointer->GetAuthGameMode<AArgusGameModeBase>();
	ARGUS_RETURN_ON_NULL_POINTER(gameModePointer, ArgusInputLog);

	const AArgusPlayerController* activePlayerController = gameModePointer->GetActivePlayerController();
	ARGUS_RETURN_ON_NULL_POINTER(activePlayerController, ArgusInputLog);

	UArgusInputManager* inputManager = activePlayerController->GetInputManager();
	ARGUS_RETURN_ON_NULL_POINTER(inputManager, ArgusInputLog);

	return inputManager;
}

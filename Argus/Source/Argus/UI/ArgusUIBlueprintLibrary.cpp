// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusUIBlueprintLibrary.h"
#include "ArgusEntity.h"
#include "ArgusGameModeBase.h"
#include "ArgusInputManager.h"
#include "ArgusLogging.h"
#include "ArgusPlayerController.h"
#include "Engine/World.h"

#if !UE_BUILD_SHIPPING
#include "ArgusECSDebugger.h"
#endif

void UArgusUIBlueprintLibrary::SetFogOfWarDynamicMaterialInstance(UMaterialInstanceDynamic* dynamicMaterialInstance)
{
	ARGUS_RETURN_ON_NULL(dynamicMaterialInstance, ArgusUnrealObjectsLog);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusUnrealObjectsLog);

	fogOfWarComponent->m_dynamicMaterialInstance = dynamicMaterialInstance;
}

bool UArgusUIBlueprintLibrary::ShouldDrawFogOfWar()
{
	bool output = true;
#if !UE_BUILD_SHIPPING
	output = ArgusECSDebugger::ShouldDrawFogOfWar();
#endif

	return output;
}

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

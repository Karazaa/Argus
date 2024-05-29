// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Engine/World.h"
#include "Systems/NavigationSystems.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TransformSystems.h"

void ArgusSystemsManager::RunSystems(TWeakObjectPtr<UWorld> worldPointer, float deltaTime)
{
	ARGUS_TRACE(ArgusSystemsManager::RunSystems)

	if (!worldPointer.IsValid())
	{
		UE_LOG
		(
			ArgusECSLog, Error, TEXT("[%s] was invoked with an invalid %s, %s."),
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(TWeakObjectPtr<UWorld>), 
			ARGUS_NAMEOF(worldPointer)
		);
		return;
	}

	NavigationSystems::RunSystems(worldPointer);
	TargetingSystems::RunSystems(deltaTime);
	TransformSystems::RunSystems(deltaTime);
}
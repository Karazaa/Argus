// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsManager.h"
#include "ArgusUtil.h"
#include "Engine/World.h"
#include "Systems/TargetingSystems.h"
#include "Systems/TransformSystems.h"

void ArgusSystemsManager::RunSystems(TWeakObjectPtr<UWorld> worldPointer, float deltaTime)
{
	if (!worldPointer.IsValid())
	{
		UE_LOG
		(
			ArgusGameLog, Error, TEXT("[%s] was invoked with an invalid %s, %s."), 
			ARGUS_FUNCNAME, 
			ARGUS_NAMEOF(TWeakObjectPtr<UWorld>), 
			ARGUS_NAMEOF(worldPointer)
		);
		return;
	}

	// TODO JAMES: Make path calculation system and run it here before targeting and transform systems.

	TargetingSystems::RunSystems(deltaTime);
	TransformSystems::RunSystems(deltaTime);
}
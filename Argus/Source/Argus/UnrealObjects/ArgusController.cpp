// Copyright Karazaa. This is a part of an RTS project called Argus.


#include "ArgusController.h"
#include "ArgusActor.h"

// Add default functionality here for any IArgusController functions that are not pure virtual.

ETeam IArgusController::GetControlledTeam()
{
	return ETeam::None;
}

bool IArgusController::HasRequiredEntities()
{
	return false;
}

TArray<TSubclassOf<AArgusActor>> IArgusController::GetRequredEntityClasses()
{
	return TArray<TSubclassOf<AArgusActor>>();
}

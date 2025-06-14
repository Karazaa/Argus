// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TaskSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void TaskSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TaskSystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
	}
}

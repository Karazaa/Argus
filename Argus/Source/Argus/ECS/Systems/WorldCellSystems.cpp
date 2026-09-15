// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "WorldCellSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "SystemArgumentDefinitions/WorldCellSystemsArgs.h"

void WorldCellSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(WorldCellSystems::RunSystems);

	ArgusIterators::IterateSystemsArgs<WorldCellSystemsArgs>([](const WorldCellSystemsArgs& components)
	{
		UpdateWorldCellLocationPerEntity(components);
	});
}

void WorldCellSystems::UpdateWorldCellLocationPerEntity(const WorldCellSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "JamesSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void JamesSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(JamesSystems::RunSystems);

	ArgusIterators::IterateEntities([](ArgusEntity entity)
	{

	});
}

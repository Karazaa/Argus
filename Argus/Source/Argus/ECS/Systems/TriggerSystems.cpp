// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TriggerSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "SystemArgumentDefinitions/TriggerSystemsArgs.h"

void TriggerSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TriggerSystems::RunSystems);

	ArgusIterators::IterateSystemsArgs<TriggerSystemsArgs>([](TriggerSystemsArgs& components)
	{

	});
}

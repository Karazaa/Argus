// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DecalSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void DecalSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(DecalSystems::RunSystems);

	DecalSystemsArgs components;
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(i)))
		{
			continue;
		}

		if (!components.m_decalComponent->m_lifetimeTimer.WasTimerSet())
		{
			components.m_decalComponent->m_lifetimeTimer.StartTimer(components.m_decalComponent->m_lifetimeSeconds);
			continue;
		}

		if (components.m_decalComponent->m_lifetimeTimer.IsTimerComplete())
		{
			components.m_entity.Destroy();
		}
	}
}

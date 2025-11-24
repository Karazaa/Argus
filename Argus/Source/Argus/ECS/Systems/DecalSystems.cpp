// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DecalSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void DecalSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(DecalSystems::RunSystems);

	ArgusEntity::IterateSystemsArgs<DecalSystemsArgs>([deltaTime](DecalSystemsArgs& components)
	{
		if (!components.m_decalComponent->m_lifetimeTimer.WasTimerSet())
		{
			components.m_decalComponent->m_lifetimeTimer.StartTimer(components.m_decalComponent->m_lifetimeSeconds);
			return;
		}

		if (components.m_decalComponent->m_lifetimeTimer.IsTimerComplete())
		{
			components.m_taskComponent->m_baseState = EBaseState::DestroyedWaitingForActorRelease;
		}
	});
}

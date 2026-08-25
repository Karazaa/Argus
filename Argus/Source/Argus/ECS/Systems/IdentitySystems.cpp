// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "IdentitySystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/SpatialPartitioningSystems.h"

void IdentitySystems::RegisterEntityAsSeenByOther(ArgusEntity perceivedEntity, ArgusEntity perceiverEntity)
{
	ARGUS_TRACE(IdentitySystems::RegisterEntityAsSeenByOther)

	if (!perceivedEntity || !perceiverEntity || !perceiverEntity.IsAlive())
	{
		return;
	}

	const IdentityComponent* perceiverIdentityComponent = perceiverEntity.GetComponent<IdentityComponent>();
	const TaskComponent* perceiverTaskComponent = perceiverEntity.GetComponent<TaskComponent>();
	IdentityComponent* perceivedIdentityComponent = perceivedEntity.GetComponent<IdentityComponent>();
	if (!perceiverIdentityComponent || !perceivedIdentityComponent)
	{
		return;
	}

	if (perceivedIdentityComponent->IsSeenBy(perceiverIdentityComponent->m_team))
	{
		return;
	}

	if (perceiverTaskComponent->m_flightState == EFlightState::Grounded && !SpatialPartitioningSystems::IsEntityInLineOfSightOfOther(perceiverEntity, perceivedEntity))
	{
		return;
	}

	perceivedIdentityComponent->AddSeenBy(perceiverIdentityComponent->m_team);
}

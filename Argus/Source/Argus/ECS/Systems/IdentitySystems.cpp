// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "IdentitySystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/SpatialPartitioningSystems.h"

void IdentitySystems::RegisterEntityAsSeenByOther(const uint16 perceivedEntityId, const uint16 perceiverEntityId)
{
	const ArgusEntity perceivedEntity = ArgusEntity::RetrieveEntity(perceivedEntityId);
	const ArgusEntity perceiverEntity = ArgusEntity::RetrieveEntity(perceiverEntityId);
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

	if (perceiverTaskComponent->m_flightState == EFlightState::Grounded && !SpatialPartitioningSystems::IsEntityInLineOfSightOfOther(perceiverEntity, perceivedEntity))
	{
		return;
	}

	perceivedIdentityComponent->AddSeenBy(perceiverIdentityComponent->m_team);
}

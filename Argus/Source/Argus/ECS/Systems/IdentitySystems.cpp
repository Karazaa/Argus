// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "IdentitySystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

//void IdentitySystems::RunSystems(float deltaTime)
//{
//	ARGUS_TRACE(IdentitySystems::RunSystems);
//
//	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
//	{
//	}
//}

void IdentitySystems::RegisterEntityAsSeenByOther(const uint16 perceivedEntityId, const uint16 perceiverEntityId)
{
	const ArgusEntity perceivedEntity = ArgusEntity::RetrieveEntity(perceivedEntityId);
	const ArgusEntity perceiverEntity = ArgusEntity::RetrieveEntity(perceiverEntityId);
	if (!perceivedEntity || !perceiverEntity || !perceiverEntity.IsAlive())
	{
		return;
	}

	const IdentityComponent* perceiverIdentityComponent = perceiverEntity.GetComponent<IdentityComponent>();
	IdentityComponent* perceivedIdentityComponent = perceivedEntity.GetComponent<IdentityComponent>();
	if (!perceiverIdentityComponent || !perceivedIdentityComponent)
	{
		return;
	}

	perceivedIdentityComponent->AddSeenBy(perceiverIdentityComponent->m_team);
}

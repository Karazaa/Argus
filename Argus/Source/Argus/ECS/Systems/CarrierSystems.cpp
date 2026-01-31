// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "CarrierSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

bool CarrierSystems::CanEntityCarryOtherEntity(ArgusEntity entity, ArgusEntity otherEntity)
{
	if (!entity.IsAlive() || !otherEntity.IsAlive())
	{
		return false;
	}

	if (!entity.IsOnSameTeamAsOtherEntity(otherEntity))
	{
		return false;
	}

	PassengerComponent* passengerComponent = otherEntity.GetComponent<PassengerComponent>();
	if (!passengerComponent)
	{
		return false;
	}

	if (passengerComponent->m_carrierEntityId != ArgusECSConstants::k_maxEntities)
	{
		return false;
	}

	CarrierComponent* carrierComponent = entity.GetComponent<CarrierComponent>();
	if (!carrierComponent)
	{
		return false;
	}

	if (carrierComponent->m_passengerEntityIds.Num() >= carrierComponent->m_carrierCapacity)
	{
		return false;
	}

	return true;
}

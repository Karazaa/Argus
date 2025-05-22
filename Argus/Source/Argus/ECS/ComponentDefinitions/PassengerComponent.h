// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct PassengerComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_OBSERVABLE_DECLARATION(uint16, m_carrierEntityId, ArgusECSConstants::k_maxEntities)
};
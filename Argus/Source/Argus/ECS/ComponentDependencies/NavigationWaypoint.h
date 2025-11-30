// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusECSConstants.h"

struct NavigationWaypoint
{
	FVector m_location = FVector::ZeroVector;
	uint16 m_decalEntityId = ArgusECSConstants::k_maxEntities;

	NavigationWaypoint() {}
	NavigationWaypoint(FVector location) : m_location(location), m_decalEntityId(ArgusECSConstants::k_maxEntities) {}
	NavigationWaypoint(FVector location, uint16 decalEntityId) : m_location(location), m_decalEntityId(decalEntityId) {}
};
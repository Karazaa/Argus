// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "CoreMinimal.h"

struct PotentialCollision
{
	uint16 m_entityID = ArgusECSConstants::k_maxEntities;
	uint16 m_navigationPointIndex = 0u;
};
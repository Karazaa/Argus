// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "../ArgusECSConstants.h"

struct TargetingComponent
{
	uint32 m_targetEntityId = ArgusECSConstants::k_maxEntities;
};
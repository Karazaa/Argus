// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "CoreMinimal.h"
#include "Misc/Optional.h"

struct SpawnEntityInfo
{
	TOptional<FVector> m_spawnLocationOverride = NullOpt;
	uint32 m_argusActorRecordId = 0u;
	uint32 m_spawningAbilityRecordId = 0u;
	float m_timeToCastSeconds = 0.0f;
	uint16 m_initiatingEntityId = ArgusECSConstants::k_maxEntities;
	bool m_needsConstruction = false;
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusMacros.h"
#include "CoreMinimal.h"

UENUM()
enum class EFlockingState : uint8
{
	Stable,
	Shrinking
};

struct FlockingComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	uint16 m_flockingRootId = ArgusECSConstants::k_maxEntities;

	// RADIUS RELATED
	ARGUS_IGNORE()
	uint16 m_numEntitiesInStableRange = 0u;

	ARGUS_IGNORE()
	float m_flockingStableRange = 100.0f;

	// TIMING RELATED
	ARGUS_IGNORE()
	float m_minDistanceFromFlockingPoint = FLT_MAX;

	ARGUS_IGNORE()
	float m_timeAtMinFlockingDistance = 0.0f;

	ARGUS_IGNORE()
	EFlockingState m_flockingState = EFlockingState::Stable;
};

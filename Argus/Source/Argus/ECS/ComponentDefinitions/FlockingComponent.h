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

	// TIMING RELATED
	ARGUS_IGNORE()
	float m_minDistanceFromFlockingPoint = FLT_MAX;

	ARGUS_IGNORE()
	float m_timeAtMinFlockingDistance = 0.0f;

	float m_maxShrinkingDurationTimeoutSeconds = 3.0f;

	// RADIUS RELATED
	float m_flockingRootRadiusIncrement = 55.0f;

	ARGUS_IGNORE()
	uint16 m_flockingRootId = ArgusECSConstants::k_maxEntities;

	ARGUS_IGNORE()
	uint16 m_concentricFlockingTier = 0u;

	ARGUS_IGNORE()
	uint16 m_numEntitiesInStableRange = 0u;

	ARGUS_IGNORE()
	EFlockingState m_flockingState = EFlockingState::Stable;
};

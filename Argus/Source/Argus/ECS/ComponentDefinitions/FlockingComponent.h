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
	ARGUS_COMP_NO_DATA
	float m_minDistanceFromFlockingPoint = FLT_MAX;

	ARGUS_COMP_NO_DATA
	float m_timeAtMinFlockingDistance = 0.0f;

	float m_maxShrinkingDurationTimeoutSeconds = 3.0f;

	// RADIUS RELATED
	float m_flockingRootRadiusIncrement = 55.0f;

	ARGUS_COMP_NO_DATA
	uint16 m_numEntitiesInStableRange = 0u;

	ARGUS_COMP_NO_DATA
	uint8 m_concentricFlockingTier = 1u;

	ARGUS_COMP_NO_DATA
	EFlockingState m_flockingState = EFlockingState::Stable;

	void ResetPackingValues()
	{
		m_numEntitiesInStableRange = 0u;
		m_concentricFlockingTier = 1u;
	}

	void ResetTimingValues()
	{
		m_minDistanceFromFlockingPoint = FLT_MAX;
		m_timeAtMinFlockingDistance = 0.0f;
	}
};

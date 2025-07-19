// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusECSConstants.h"
#include "ArgusMacros.h"

UENUM()
enum class EAvoidancePriority : uint8
{
	Lowest,
	Low,
	Medium,
	High,
	Highest
};

UENUM()
enum class EFlockingState : uint8
{
	Stable,
	Shrinking
};

struct AvoidanceGroupingComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	FVector m_groupAverageLocation = FVector::ZeroVector;

	ARGUS_IGNORE()
	uint16 m_groupId = ArgusECSConstants::k_maxEntities;

	ARGUS_IGNORE()
	uint16 m_numberOfIdleEntities = 0u;

	EAvoidancePriority m_avoidancePriority = EAvoidancePriority::Lowest;

	ARGUS_IGNORE()
	EFlockingState m_flockingState = EFlockingState::Stable;
};
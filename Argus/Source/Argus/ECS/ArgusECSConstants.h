// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"

namespace ArgusECSConstants
{
	static constexpr uint16	k_maxEntities = 15000u;

	static constexpr float k_pathFindingAgentRadius = 45.0f;
	static constexpr float k_pathFindingAgentHeight = 100.0f;
	static constexpr float k_avoidanceCollisionDetectionPredictionTime = 1.25f;
	static constexpr float k_avoidanceAgentSearchRadius = 250.0f;
	static constexpr float k_avoidanceEpsilonValue = 0.00001f;
	static constexpr float k_moveAlongPathWaypointTolerance = 0.001f;
}

// Unreal complains like a dumbass if UENUM is in a namespace. Big dumb.
UENUM()
enum class UEntityPriority : uint16
{
	HighestPriority = 0,
	HighPriority = 1000u,
	MediumPriority = 3000u,
	LowPriority = 6000u,
	LowestPriority = 10000u,
};
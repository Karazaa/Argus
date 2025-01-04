// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"

namespace ArgusECSConstants
{
	static constexpr uint16	k_maxEntities = 15000u;
	static constexpr uint16	k_singletonEntityId = k_maxEntities - 1u;

	static constexpr float k_avoidanceEntityDetectionPredictionTime = 1.0f;
	static constexpr float k_avoidanceObstacleDetectionPredictionTime = 0.5f;
	static constexpr float k_avoidanceAgentSearchRadius = 200.0f;
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
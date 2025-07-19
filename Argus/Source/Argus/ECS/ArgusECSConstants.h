// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"

namespace ArgusECSConstants
{
	static constexpr uint16	k_maxEntities = 15000u;
	static constexpr uint16	k_singletonEntityId = k_maxEntities - 1u;

	static constexpr float k_avoidanceEntityDetectionPredictionTime = 2.0f;
	static constexpr float k_avoidanceObstacleDetectionPredictionTime = 2.0f;
	static constexpr float k_avoidanceObstacleSplitDistance = 150.0f;
	static constexpr float k_avoidanceObstacleQueryRadiusMultiplier = 1.5f;
	static constexpr float k_avoidanceAgentAdditionalBufferRadius = 25.0f;
	static constexpr float k_avoidanceAgentReturnToEndNavRadius = 10.0f;
	static constexpr float k_avoidanceEpsilonValue = 0.00001f;
	static constexpr float k_moveAlongPathWaypointTolerance = 0.001f;
	static constexpr float k_flockingRangeExtension = 2.0f;

	static constexpr int32 k_maxDetourWalls = 1028;
	static constexpr int32 k_maxDetourPolys = 1028;
	static constexpr float k_detourQuerySize = 3000.0f;

	static constexpr int32 k_initialResourcesQuantity = 1000;

	static constexpr float k_debugDrawLineWidth = 3.0f;
	static constexpr float k_debugDrawHeightAdjustment = 5.0f;
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
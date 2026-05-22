// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/SettingsStructures.h"
#include "CoreMinimal.h"

struct GlobalSettingsComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Avoidance")) ARGUS_COMP_TRANSIENT
	float m_avoidanceEntityDetectionPredictionTime = 0.75f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Avoidance")) ARGUS_COMP_TRANSIENT
	float m_avoidanceObstacleDetectionPredictionTime = 0.75f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Avoidance")) ARGUS_COMP_TRANSIENT
	float m_maxAvoidanceObstaclePointHeightDifference = 50.0f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Flocking")) ARGUS_COMP_TRANSIENT
	float m_flockingVelocityInfluence = 0.3f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Obstacles")) ARGUS_COMP_TRANSIENT
	float m_obstacleShrinkFixupDistance = 25.0f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Obstacles")) ARGUS_COMP_TRANSIENT
	float m_maxObstaclePointDistance = 150.0f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Obstacles")) ARGUS_COMP_TRANSIENT
	float m_minObstaclePointDistance = 75.0f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Translation")) ARGUS_COMP_TRANSIENT
	float m_progressNavPathDistThreshold = 5.0f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Navigation")) ARGUS_COMP_TRANSIENT ARGUS_COMP_NO_RESET
	TArray<FGroupSizeRadiusPair> m_groupSizeRadiusPair;

	static const GlobalSettingsComponent* Get();
};

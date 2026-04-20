// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct GlobalSettingsComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Avoidance")) ARGUS_COMP_TRANSIENT
	float m_avoidanceEntityDetectionPredictionTime = 0.75f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Avoidance")) ARGUS_COMP_TRANSIENT
	float m_avoidanceObstacleDetectionPredictionTime = 0.75f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Flocking")) ARGUS_COMP_TRANSIENT
	float m_flockingVelocityInfluence = 0.3f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Obstacles")) ARGUS_COMP_TRANSIENT
	float m_obstacleShrinkFixupDistance = 25.0f;

	ARGUS_COMP_PROPERTY(EditDefaultsOnly, meta = (Category = "Obstacles")) ARGUS_COMP_TRANSIENT
	float m_maxObstaclePointDistance = 150.0f;

	static const GlobalSettingsComponent* Get();
};

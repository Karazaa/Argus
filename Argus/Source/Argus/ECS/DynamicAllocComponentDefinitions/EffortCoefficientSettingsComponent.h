// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct EffortCoefficientSettingsComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Same Avoidance Group")) ARGUS_COMP_TRANSIENT
	float m_sameAvoidanceGroupBase = 0.5f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Same Avoidance Group")) ARGUS_COMP_TRANSIENT
	float m_sameAvoidanceGroupIsAttacking = 0.33f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Same Avoidance Group")) ARGUS_COMP_TRANSIENT
	float m_sameAvoidanceGroupIsConstructing = 0.33f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Adjacent to Obstacles")) ARGUS_COMP_TRANSIENT
	float m_sameTeamBothObstacle = 0.4f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Adjacent to Obstacles")) ARGUS_COMP_TRANSIENT
	float  m_sameTeamHasObstacle = 0.0f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Adjacent to Obstacles")) ARGUS_COMP_TRANSIENT
	float m_sameTeamOtherHasObstacle = 1.0f;

	static const EffortCoefficientSettingsComponent* Get();
};

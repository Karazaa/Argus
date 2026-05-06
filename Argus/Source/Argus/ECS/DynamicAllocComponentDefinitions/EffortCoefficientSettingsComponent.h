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
	float m_sameAvoidanceGroupBothObstacle = 0.3f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Same Avoidance Group")) ARGUS_COMP_TRANSIENT
	float m_sameAvoidanceGroupHasObstacle = 0.0f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Same Avoidance Group")) ARGUS_COMP_TRANSIENT
	float m_sameAvoidanceGroupOtherHasObstacle = 0.75f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Same Avoidance Group")) ARGUS_COMP_TRANSIENT
	float m_sameAvoidanceGroupIsAttacking = 0.33f;

	ARGUS_COMP_PROPERTY(EditAnywhere, meta = (Category = "Same Avoidance Group")) ARGUS_COMP_TRANSIENT
	float m_sameAvoidanceGroupIsConstructing = 0.33f;

	static const EffortCoefficientSettingsComponent* Get();
};

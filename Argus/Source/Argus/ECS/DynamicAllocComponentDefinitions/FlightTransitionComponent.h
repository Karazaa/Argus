// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "CoreMinimal.h"

struct FlightTransitionComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ArgusMath::SecondOrderSystemSmoother<float> m_smoothedTransitionAltitude;

	ARGUS_COMP_NO_DATA
	float m_targetTransitionAltitude = 0.0f;

	ARGUS_COMP_NO_DATA
	uint32 m_onTransitionCompleteAbilityId = 0u;
};

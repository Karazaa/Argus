// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "CoreMinimal.h"

struct FacingComponent
{
	ARGUS_COMPONENT_SHARED;

	ArgusMath::ExponentialDecaySmoother<float> m_smoothedYaw;

	ARGUS_COMP_NO_DATA
	float m_targetYaw = 0.0f;

	float GetCurrentYaw() const
	{
		return m_smoothedYaw.GetValue();
	}
};

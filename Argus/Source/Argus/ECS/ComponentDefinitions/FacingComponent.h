// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "CoreMinimal.h"

struct FacingComponent
{
	ARGUS_COMPONENT_SHARED;

	ArgusMath::SecondOrderSystemSmoother<float> m_smoothedFacing;

	ARGUS_COMP_NO_DATA
	float m_targetFacing = 0.0f;

	float GetCurrentFacing() const
	{
		return m_smoothedFacing.GetValue();
	}
};

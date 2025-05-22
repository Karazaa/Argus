// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct VelocityComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	FVector m_currentVelocity = FVector::ZeroVector;

	ARGUS_IGNORE()
	FVector m_proposedAvoidanceVelocity = FVector::ZeroVector;

	float m_desiredSpeedUnitsPerSecond = 100.0f;
};

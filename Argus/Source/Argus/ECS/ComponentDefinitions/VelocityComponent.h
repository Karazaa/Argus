// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct VelocityComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	FVector2D m_currentVelocity = FVector2D::ZeroVector;

	ARGUS_IGNORE()
	FVector2D m_proposedAvoidanceVelocity = FVector2D::ZeroVector;

	float m_desiredSpeedUnitsPerSecond = 100.0f;
	float m_desiredFlightSpeedUnitsPerSecond = 100.0f;
};

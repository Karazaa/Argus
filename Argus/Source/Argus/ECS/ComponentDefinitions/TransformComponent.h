// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "CoreMinimal.h"

UENUM()
enum class EFlightCapability : uint8
{
	OnlyGrounded,
	OnlyFlying,
	BothGroundedAndFlying
};

struct TransformComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	FVector m_location = FVector::ZeroVector;

	ArgusMath::ExponentialDecaySmoother<float> m_smoothedYaw;

	ARGUS_IGNORE()
	float m_targetYaw = 0.0f;

	float m_radius = 45.0f;
	float m_height = 100.0f;
	EFlightCapability m_flightCapability;

	float GetCurrentYaw() const
	{
		return m_smoothedYaw.GetValue();
	}
};
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
	ARGUS_COMPONENT_SHARED

	ARGUS_COMP_NO_DATA
	FVector m_location = FVector::ZeroVector;

	ArgusMath::ExponentialDecaySmoother<float> m_smoothedTransitionAltitude;

	ARGUS_COMP_NO_DATA
	float m_targetTransitionAltitude = 0.0f;

	float m_radius = 45.0f;
	
	EFlightCapability m_flightCapability;

	bool CanFly() const
	{
		return m_flightCapability == EFlightCapability::BothGroundedAndFlying || m_flightCapability == EFlightCapability::OnlyFlying;
	}

	bool CanWalk() const
	{
		return m_flightCapability == EFlightCapability::BothGroundedAndFlying || m_flightCapability == EFlightCapability::OnlyGrounded;
	}
};
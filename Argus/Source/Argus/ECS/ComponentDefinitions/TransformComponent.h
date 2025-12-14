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

	ArgusMath::ExponentialDecaySmoother<float> m_smoothedTransitionAltitude;

	ARGUS_IGNORE()
	float m_targetYaw = 0.0f;

	ARGUS_IGNORE()
	float m_targetTransitionAltitude = 0.0f;

	float m_radius = 45.0f;
	
	EFlightCapability m_flightCapability;

	float GetCurrentYaw() const
	{
		return m_smoothedYaw.GetValue();
	}

	bool CanFly() const
	{
		return m_flightCapability == EFlightCapability::BothGroundedAndFlying || m_flightCapability == EFlightCapability::OnlyFlying;
	}

	bool CanWalk() const
	{
		return m_flightCapability == EFlightCapability::BothGroundedAndFlying || m_flightCapability == EFlightCapability::OnlyGrounded;
	}
};
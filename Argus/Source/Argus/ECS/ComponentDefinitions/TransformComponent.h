// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "CoreMinimal.h"

struct TransformComponent
{
	ARGUS_COMPONENT_SHARED

	ARGUS_IGNORE()
	FVector m_location = FVector::ZeroVector;

	ARGUS_IGNORE()
	FVector m_avoidanceGroupSourceLocation = FVector::ZeroVector;

	ARGUS_IGNORE()
	FVector m_currentVelocity = FVector::ZeroVector;

	ARGUS_IGNORE()
	FVector m_proposedAvoidanceVelocity = FVector::ZeroVector;

	ArgusMath::ExponentialDecaySmoother<float> m_smoothedYaw;

	ARGUS_IGNORE()
	float m_targetYaw;

	float m_desiredSpeedUnitsPerSecond = 100.0f;
	float m_radius = 45.0f;
	float m_height = 100.0f;

	float GetCurrentYaw() const
	{
		return m_smoothedYaw.GetValue();
	}

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (Location: {%f, %f, %f}) \n    (%s: %f) \n    (%s: %f)"),
				ARGUS_NAMEOF(TransformComponent),
				m_location.X,
				m_location.Y,
				m_location.Z,
				ARGUS_NAMEOF(m_targetYaw),
				m_targetYaw,
				ARGUS_NAMEOF(m_desiredSpeedUnitsPerSecond),
				m_desiredSpeedUnitsPerSecond
			)
		);
	}
};
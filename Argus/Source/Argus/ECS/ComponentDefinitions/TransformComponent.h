// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct TransformComponent
{
	ARGUS_IGNORE()
	FTransform m_transform = FTransform::Identity;

	ARGUS_IGNORE()
	FVector m_currentVelocity = FVector::ZeroVector;

	ARGUS_IGNORE()
	FVector m_proposedAvoidanceVelocity = FVector::ZeroVector;

	float m_desiredSpeedUnitsPerSecond = 100.0f;
	float m_radius = 45.0f;
	float m_height = 100.0f;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		FVector location = m_transform.GetLocation();

		debugStringToAppendTo.Append
		(
			FString::Printf
			(
				TEXT("\n[%s] \n    (Location: {%f, %f, %f}) \n    (%s: %f)"),
				ARGUS_NAMEOF(TransformComponent),
				location.X,
				location.Y,
				location.Z,
				ARGUS_NAMEOF(m_desiredSpeedUnitsPerSecond),
				m_desiredSpeedUnitsPerSecond
			)
		);
	}
};
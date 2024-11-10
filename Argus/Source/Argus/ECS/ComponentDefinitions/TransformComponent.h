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

	void GetDebugString(FString& debugStringToAppendTo) const
	{
		debugStringToAppendTo.Append(FString::Printf(TEXT("\n[%s]"), ARGUS_NAMEOF(TransformComponent)));
	}
};
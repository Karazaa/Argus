// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct TransformComponent
{
	ARGUS_IGNORE()
	FTransform m_transform = FTransform::Identity;

	ARGUS_IGNORE()
	float m_avoidanceSpeedUnitsPerSecond = 100.0f;
	float m_desiredSpeedUnitsPerSecond = 100.0f;
};
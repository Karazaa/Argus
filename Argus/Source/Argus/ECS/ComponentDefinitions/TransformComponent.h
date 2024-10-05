// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct TransformComponent
{
	ARGUS_IGNORE()
	FTransform m_transform = FTransform::Identity;

	float  m_speedUnitsPerSecond = 100.0f;
};
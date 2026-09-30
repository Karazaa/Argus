// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct RelativeCircleComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	FVector2D m_relativeCenter = FVector2D::ZeroVector;

	float m_radius = 100.0f;
};

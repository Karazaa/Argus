// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

#include "RelativePolygonComponent.generated.h"

UENUM()
enum class ERelativePolygonType : uint8
{
	TriggerVolume,
	Obstacle
};

struct RelativePolygonComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	TArray<FVector2D> m_relativeUnrealVerticies;

	ERelativePolygonType m_polygonType = ERelativePolygonType::TriggerVolume;
};

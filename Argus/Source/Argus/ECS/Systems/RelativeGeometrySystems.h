// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

struct FacingComponent;
struct RelativeCircleComponent;
struct RelativePolygonComponent;
struct TransformComponent;

class RelativeGeometrySystems
{
public:
	static FVector GetWorldSpaceRelativeCircleCenter(const RelativeCircleComponent* relativeCircleComponent, const FacingComponent* facingComponent, const TransformComponent* transformComponent);
	static void GetWorldSpaceRelativePolygonPoints(const RelativePolygonComponent* relativePolygonComponent, const FacingComponent* facingComponent, const TransformComponent* transformComponent, TArray<FVector>& outWorldSpacePoints);
};

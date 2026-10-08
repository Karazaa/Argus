// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RelativeGeometrySystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

FVector RelativeGeometrySystems::GetWorldSpaceRelativeCircleCenter(const RelativeCircleComponent* relativeCircleComponent, const FacingComponent* facingComponent, const TransformComponent* transformComponent)
{
	ARGUS_TRACE(RelativeGeometrySystems::GetWorldSpaceRelativeCircleCenter);
	
	ARGUS_RETURN_ON_NULL_VALUE(relativeCircleComponent, ArgusECSLog, FVector::ZeroVector);
	ARGUS_RETURN_ON_NULL_VALUE(facingComponent, ArgusECSLog, FVector::ZeroVector);
	ARGUS_RETURN_ON_NULL_VALUE(transformComponent, ArgusECSLog, FVector::ZeroVector);

	// TODO JAMES: Use facing and location to calculate the worldspace center of the relative circle.

	return FVector::ZeroVector;
}

void RelativeGeometrySystems::GetWorldSpaceRelativePolygonPoints(const RelativePolygonComponent* relativePolygonComponent, const FacingComponent* facingComponent, const TransformComponent* transformComponent, TArray<FVector>& outWorldSpacePoints)
{
	ARGUS_TRACE(RelativeGeometrySystems::GetWorldSpaceRelativePolygonPoints);

	ARGUS_RETURN_ON_NULL(relativePolygonComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(facingComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(transformComponent, ArgusECSLog);

	// TODO JAMES: Use facing and location to calculate the worldspace points of the relative polygon.
}
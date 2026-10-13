// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RelativeGeometrySystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"

FVector RelativeGeometrySystems::GetWorldSpaceRelativeCircleCenter(const RelativeCircleComponent* relativeCircleComponent, const FacingComponent* facingComponent, const TransformComponent* transformComponent)
{
	ARGUS_TRACE(RelativeGeometrySystems::GetWorldSpaceRelativeCircleCenter);
	
	ARGUS_RETURN_ON_NULL_VALUE(relativeCircleComponent, ArgusECSLog, FVector::ZeroVector);
	ARGUS_RETURN_ON_NULL_VALUE(facingComponent, ArgusECSLog, FVector::ZeroVector);
	ARGUS_RETURN_ON_NULL_VALUE(transformComponent, ArgusECSLog, FVector::ZeroVector);

	const FVector forwardVector = ArgusMath::GetDirectionFromYaw(facingComponent->m_smoothedFacing.GetValue());
	const FTransform basisTransform = FTransform(FRotationMatrix::MakeFromXZ(forwardVector, FVector::UpVector).ToQuat(), transformComponent->m_location);

	return basisTransform.TransformPosition(FVector(relativeCircleComponent->m_relativeCenter, 0.0f));
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "Camera/CameraComponent.h"

AArgusCameraActor::AArgusCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent")));
}

void AArgusCameraActor::UpdateCamera(FVector2D screenSpaceMousePosition, FVector2D screenSpaceXYBounds, float deltaTime)
{
	const float paddingAmountX = screenSpaceXYBounds.X * m_screenMovePaddingProportion;
	const float scaledVelocity = m_cameraMoveVelocity * deltaTime;
	const float paddingAmountY = screenSpaceXYBounds.Y * m_screenMovePaddingProportion;

	const FVector currentLocation = GetActorLocation();
	FVector translation = FVector::ZeroVector;
	
	if (screenSpaceMousePosition.X < paddingAmountX)
	{
		translation += m_moveRightDir * (-1.0f * scaledVelocity);
	}
	else if (screenSpaceMousePosition.X > screenSpaceXYBounds.X - paddingAmountX)
	{
		translation += m_moveRightDir * scaledVelocity;
	}

	if (screenSpaceMousePosition.Y < paddingAmountY)
	{
		translation += m_moveUpDir * scaledVelocity;
	}
	else if (screenSpaceMousePosition.Y > screenSpaceXYBounds.Y - paddingAmountY)
	{
		translation += m_moveUpDir * (-1.0f * scaledVelocity);
	}

	SetActorLocation(currentLocation + translation);
}
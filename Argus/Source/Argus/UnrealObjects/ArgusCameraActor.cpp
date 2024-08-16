// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "ArgusMacros.h"
#include "Camera/CameraComponent.h"

AArgusCameraActor::AArgusCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent")));
}

void AArgusCameraActor::UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraPanning)

	if (!cameraParameters.m_screenSpaceMousePosition || !cameraParameters.m_screenSpaceXYBounds)
	{
		return;
	}

	const float paddingAmountX = cameraParameters.m_screenSpaceXYBounds->X * m_screenMovePaddingProportion;
	const float scaledVelocity = m_cameraMoveVelocity * deltaTime;
	const float paddingAmountY = cameraParameters.m_screenSpaceXYBounds->Y * m_screenMovePaddingProportion;

	const FVector currentLocation = GetActorLocation();
	FVector translation = FVector::ZeroVector;
	
	if (cameraParameters.m_screenSpaceMousePosition->X < paddingAmountX)
	{
		translation += m_moveRightDir * (-1.0f * scaledVelocity);
	}
	else if (cameraParameters.m_screenSpaceMousePosition->X > (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX))
	{
		translation += m_moveRightDir * scaledVelocity;
	}

	if (cameraParameters.m_screenSpaceMousePosition->Y < paddingAmountY)
	{
		translation += m_moveUpDir * scaledVelocity;
	}
	else if (cameraParameters.m_screenSpaceMousePosition->Y > (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY))
	{
		translation += m_moveUpDir * (-1.0f * scaledVelocity);
	}

	SetActorLocation(currentLocation + translation);
}

void AArgusCameraActor::UpdateCameraZoom(float inputZoomValue)
{

}

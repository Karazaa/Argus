// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "ArgusMacros.h"
#include "Camera/CameraComponent.h"

AArgusCameraActor::AArgusCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent")));
}

void AArgusCameraActor::UpdateCamera(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCamera)

	UpdateCameraPanning(cameraParameters, deltaTime);
	UpdateCameraZoomInternal(deltaTime);

	m_zoomInputThisFrame = 0.0f;
}

void AArgusCameraActor::UpdateCameraZoom(const float inputZoomValue)
{
	m_zoomInputThisFrame += inputZoomValue;
}

void AArgusCameraActor::BeginPlay()
{
	Super::BeginPlay();
	m_cameraPositionWithoutZoom = GetActorLocation();
}

void AArgusCameraActor::UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraPanning)

	if (!cameraParameters.m_screenSpaceMousePosition || !cameraParameters.m_screenSpaceXYBounds)
	{
		return;
	}

	const float paddingAmountX = cameraParameters.m_screenSpaceXYBounds->X * m_horizontalScreenMovePaddingProportion;
	const float paddingAmountY = cameraParameters.m_screenSpaceXYBounds->Y * m_verticalScreenMovePaddingProportion;
	float desiredVerticalVelocity = 0.0f;
	float desiredHorizontalVelocity = 0.0f;

	FVector translation = FVector::ZeroVector;

	// UP
	if (cameraParameters.m_screenSpaceMousePosition->Y < paddingAmountY)
	{
		desiredVerticalVelocity = m_desiredVerticalVelocity;
	}
	// DOWN
	else if (cameraParameters.m_screenSpaceMousePosition->Y > (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY))
	{
		desiredVerticalVelocity = -m_desiredVerticalVelocity;
	}
	m_currentVerticalVelocity = FMath::FInterpTo(m_currentVerticalVelocity, desiredVerticalVelocity, deltaTime, m_verticalAcceleration);
	translation += m_moveUpDir * (m_currentVerticalVelocity * deltaTime);

	// LEFT
	if (cameraParameters.m_screenSpaceMousePosition->X < paddingAmountX)
	{
		desiredHorizontalVelocity = -m_desiredHorizontalVelocity;
	}
	// RIGHT
	else if (cameraParameters.m_screenSpaceMousePosition->X > (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX))
	{
		desiredHorizontalVelocity = m_desiredHorizontalVelocity;
	}
	m_currentHorizontalVelocity = FMath::FInterpTo(m_currentHorizontalVelocity, desiredHorizontalVelocity, deltaTime, m_horizontalAcceleration);
	translation += m_moveRightDir * (m_currentHorizontalVelocity * deltaTime);

	m_cameraPositionWithoutZoom += translation;
}

void AArgusCameraActor::UpdateCameraZoomInternal(const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraZoom)

	if (!FMath::IsNearlyZero(FMath::Sign(m_zoomInputThisFrame)))
	{
		m_zoomTargetTranslation += GetActorForwardVector() * m_zoomInputThisFrame * m_desiredZoomVelocity;
	}

	SetActorLocation(FMath::VInterpTo(GetActorLocation(), m_cameraPositionWithoutZoom + m_zoomTargetTranslation, deltaTime, m_zoomAcceleration));
}

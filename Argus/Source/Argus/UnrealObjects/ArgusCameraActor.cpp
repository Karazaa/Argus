// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Camera/CameraComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"

AArgusCameraActor::AArgusCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent")));
}

void AArgusCameraActor::ForceSetCameraPositionWithoutZoom(const FVector& position)
{
	m_cameraPositionWithoutZoom = position;
	m_currentVerticalVelocity = 0.0f;
	m_currentHorizontalVelocity = 0.0f;
	SetActorLocation(position);
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
	UWorld* world = GetWorld();
	if (!world)
	{
		UE_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld));
		return;
	}

	FVector proposedZoomUpdateThisFrame = FVector::ZeroVector;
	FVector proposedPosition = m_cameraPositionWithoutZoom + m_zoomTargetTranslation;
	const FVector forwardVector = GetActorForwardVector();
	const FVector traceEndpoint = proposedPosition + (forwardVector * AArgusCameraActor::k_cameraTraceLength);
	if (!FMath::IsNearlyZero(FMath::Sign(m_zoomInputThisFrame)))
	{
		proposedZoomUpdateThisFrame = forwardVector * m_zoomInputThisFrame * m_desiredZoomVelocity;
	}

	FHitResult hitResult;
	if (world->LineTraceSingleByChannel(hitResult, proposedPosition, traceEndpoint, ECC_WorldStatic))
	{
		const float distanceUpdateThisFrame = -m_zoomInputThisFrame * m_desiredZoomVelocity;

		// Camera is too close to terrain
		if ((distanceUpdateThisFrame + hitResult.Distance) < m_minZoomDistanceToGround)
		{
			proposedPosition = hitResult.Location + (-forwardVector * m_minZoomDistanceToGround);
			m_zoomTargetTranslation = proposedPosition - m_cameraPositionWithoutZoom;
		}
		// Camera is too far from terrain
		else if ((distanceUpdateThisFrame + hitResult.Distance) > m_maxZoomDistanceToGround)
		{
			proposedPosition = hitResult.Location + (-forwardVector * m_maxZoomDistanceToGround);
			m_zoomTargetTranslation = proposedPosition - m_cameraPositionWithoutZoom;
		}
		// Camera is neither too far nor too close to terrain
		else
		{
			m_zoomTargetTranslation += proposedZoomUpdateThisFrame;
			proposedPosition = m_cameraPositionWithoutZoom + m_zoomTargetTranslation;
		}
	}
	// Camera can't find any terrain
	else
	{
		m_zoomTargetTranslation += proposedZoomUpdateThisFrame;
		proposedPosition = m_cameraPositionWithoutZoom + m_zoomTargetTranslation;
	}

	SetActorLocation(FMath::VInterpTo(GetActorLocation(), proposedPosition, deltaTime, m_zoomAcceleration));
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Camera/CameraComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"

AArgusCameraActor::AArgusCameraActor() :	
	m_currentVerticalVelocity(ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_verticalVelocitySmoothingDecayConstant)),
	m_currentHorizontalVelocity(ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_horizontalVelocitySmoothingDecayConstant)),
	m_currentLocation(ArgusMath::ExponentialDecaySmoother<FVector>(FVector::ZeroVector, m_zoomLocationSmoothingDecayConstant))
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent")));

	m_zoomRange = TRange<float>(m_minZoomDistanceToGround, m_maxZoomDistanceToGround);
	m_zeroToOne = TRange<float>(0.0f, 1.0f);
}

void AArgusCameraActor::ForceSetCameraPositionWithoutZoom(const FVector& position)
{
	m_cameraPositionWithoutZoom = position;
	m_currentVerticalVelocity.ResetZero();
	m_currentHorizontalVelocity.ResetZero();
	m_currentLocation.Reset(position);
	SetActorLocation(position);
}

void AArgusCameraActor::UpdateCamera(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCamera);

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
	ForceSetCameraPositionWithoutZoom(GetActorLocation());
}

void AArgusCameraActor::UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraPanning);

	if (!cameraParameters.m_screenSpaceMousePosition || !cameraParameters.m_screenSpaceXYBounds)
	{
		return;
	}

	const float paddingAmountX = cameraParameters.m_screenSpaceXYBounds->X * m_horizontalScreenMovePaddingProportion;
	const float paddingAmountY = cameraParameters.m_screenSpaceXYBounds->Y * m_verticalScreenMovePaddingProportion;
	float desiredVerticalVelocity = 0.0f;
	float desiredHorizontalVelocity = 0.0f;

	FVector translation = FVector::ZeroVector;
	const float scaledDesiredVerticalVelocity = FMath::Lerp(m_minimumDesiredVerticalVelocity, m_maximumDesiredVerticalVelocity, m_zoomLevelInterpolant);
	const float scaledDesiredHorizontalVelocity = FMath::Lerp(m_minimumDesiredHorizontalVelocity, m_maximumDesiredHorizontalVelocity, m_zoomLevelInterpolant);

	// UP
	if (cameraParameters.m_screenSpaceMousePosition->Y < paddingAmountY)
	{
		desiredVerticalVelocity = scaledDesiredVerticalVelocity;
	}
	// DOWN
	else if (cameraParameters.m_screenSpaceMousePosition->Y > (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY))
	{
		desiredVerticalVelocity = -scaledDesiredVerticalVelocity;
	}
	m_currentVerticalVelocity.SmoothChase(desiredVerticalVelocity, deltaTime);
	translation += m_moveUpDir * (m_currentVerticalVelocity.GetValue() * deltaTime);

	// LEFT
	if (cameraParameters.m_screenSpaceMousePosition->X < paddingAmountX)
	{
		desiredHorizontalVelocity = -scaledDesiredHorizontalVelocity;
	}
	// RIGHT
	else if (cameraParameters.m_screenSpaceMousePosition->X > (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX))
	{
		desiredHorizontalVelocity = scaledDesiredHorizontalVelocity;
	}
	m_currentHorizontalVelocity.SmoothChase(desiredHorizontalVelocity, deltaTime);
	translation += m_moveRightDir * (m_currentHorizontalVelocity.GetValue() * deltaTime);

	m_cameraPositionWithoutZoom += translation;
}

void AArgusCameraActor::UpdateCameraZoomInternal(const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraZoom);
	UWorld* world = GetWorld();
	if (!world)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld));
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

		m_zoomLevelInterpolant = FMath::GetMappedRangeValueClamped(m_zoomRange, m_zeroToOne, hitResult.Distance);

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

	m_currentLocation.SmoothChase(proposedPosition, deltaTime);
	SetActorLocation(m_currentLocation.GetValue());
}

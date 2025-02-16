// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "Camera/CameraComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Misc/Optional.h"

uint8 AArgusCameraActor::s_numWidgetPanningBlockers = 0u;

void AArgusCameraActor::IncrementPanningBlockers()
{
	s_numWidgetPanningBlockers++;
}

void AArgusCameraActor::DecrementPanningBlockers()
{
	s_numWidgetPanningBlockers--;
}

AArgusCameraActor::AArgusCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_LastDemotable;
	SetRootComponent(CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent")));

	m_zoomRange = TRange<float>(m_minZoomDistanceToGround, m_maxZoomDistanceToGround);
	m_zeroToOne = TRange<float>(0.0f, 1.0f);
}

void AArgusCameraActor::ForceSetCameraPositionWithoutZoom(const FVector& position)
{
	m_cameraPositionWithoutZoom = position;
	m_currentVerticalVelocity.ResetZero();
	m_currentHorizontalVelocity.ResetZero();
	m_currentZoomTranslationAmount.ResetZero();
	SetActorLocation(position);
}

void AArgusCameraActor::UpdateCamera(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCamera);

	TOptional<FHitResult> hitResult = NullOpt;
	TraceToGround(hitResult);
	UpdateCameraOrbitInternal(hitResult, deltaTime);
	UpdateCameraPanning(cameraParameters, deltaTime);
	UpdateCameraZoomInternal(hitResult, deltaTime);

	m_zoomInputThisFrame = 0.0f;
}

void AArgusCameraActor::UpdateCameraOrbit(const float inputOrbitValue)
{
	m_orbitInputThisFrame += inputOrbitValue;
}

void AArgusCameraActor::UpdateCameraZoom(const float inputZoomValue)
{
	m_zoomInputThisFrame += inputZoomValue;
}

void AArgusCameraActor::BeginPlay()
{
	Super::BeginPlay();
	m_currentVerticalVelocity = ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_verticalVelocitySmoothingDecayConstant);
	m_currentHorizontalVelocity = ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_horizontalVelocitySmoothingDecayConstant);
	m_currentZoomTranslationAmount = ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_zoomLocationSmoothingDecayConstant);

	ForceSetCameraPositionWithoutZoom(GetActorLocation());

	TOptional<FHitResult> hitResult = NullOpt;
	TraceToGround(hitResult);
	if (hitResult)
	{
		const FVector2D hitLocation2D = FVector2D(hitResult.GetValue().Location);
		const FVector2D cameraLocation2D = FVector2D(m_cameraPositionWithoutZoom);
		const FVector2D unitCircleLocation = ArgusMath::ToCartesianVector2((cameraLocation2D - hitLocation2D).GetSafeNormal());
		float arcsine = FMath::Asin(unitCircleLocation.Y);
		float arccosine = FMath::Acos(unitCircleLocation.X);
		if (arcsine >= 0.0f)
		{
			m_currentOrbitTheta = arccosine;
		}
		else if (arccosine < UE_HALF_PI)
		{
			m_currentOrbitTheta = UE_TWO_PI + arcsine;
		}
		else
		{
			m_currentOrbitTheta = UE_PI - arcsine;
		}
	}
	else
	{
		m_currentOrbitTheta = UE_PI;
	}
}

void AArgusCameraActor::UpdateCameraOrbitInternal(const TOptional<FHitResult>& hitResult, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraOrbitInternal);

	if (!hitResult || m_orbitInputThisFrame == 0.0f)
	{
		return;
	}

	// Set camera position without zoom
	const FVector2D hitResultLocation = FVector2D(hitResult.GetValue().Location);
	const float radius = FVector2D::Distance(hitResultLocation, FVector2D(m_cameraPositionWithoutZoom));
	const float thetaChangeThisFrame = m_orbitInputThisFrame * deltaTime;
	m_currentOrbitTheta += thetaChangeThisFrame;

	FVector2D updatedLocation = FVector2D();
	updatedLocation.X = FMath::Cos(m_currentOrbitTheta);
	updatedLocation.Y = FMath::Sin(m_currentOrbitTheta);
	updatedLocation = ArgusMath::ToUnrealVector2(updatedLocation);

	m_moveUpDir = FVector(-updatedLocation, 0.0f);

	updatedLocation *= radius;
	m_cameraPositionWithoutZoom.X = hitResultLocation.X + updatedLocation.X;
	m_cameraPositionWithoutZoom.Y = hitResultLocation.Y + updatedLocation.Y;

	// Counter rotate camera actor.
	const FVector forwardVector = hitResult.GetValue().Location - m_cameraPositionWithoutZoom;
	const FVector rightVector = -forwardVector.Cross(FVector::UpVector);
	SetActorRotation(FRotationMatrix::MakeFromXY(forwardVector, rightVector).ToQuat());

	m_moveRightDir = rightVector.GetSafeNormal();
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
	const float minVerticalVelocityBondaryModifier = 1.0f - m_verticalVelocityBoundaryModifier;
	const float maxVerticalVelocityBondaryModifier = 1.0f + m_verticalVelocityBoundaryModifier;
	const float minHorizontalVelocityBondaryModifier = 1.0f - m_horizontalVelocityBoundaryModifier;
	const float maxHorizontalVelocityBondaryModifier = 1.0f + m_horizontalVelocityBoundaryModifier;

	float verticalModification = 1.0f;
	if (!IsPanningBlocked())
	{
		// UP
		if (cameraParameters.m_screenSpaceMousePosition->Y < paddingAmountY)
		{
			desiredVerticalVelocity = scaledDesiredVerticalVelocity;
			const float interpolator = 1.0f - ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMousePosition->Y, paddingAmountY);
			verticalModification = FMath::Lerp(minVerticalVelocityBondaryModifier, maxVerticalVelocityBondaryModifier, interpolator);
		}
		// DOWN
		else if (cameraParameters.m_screenSpaceMousePosition->Y > (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY))
		{
			desiredVerticalVelocity = -scaledDesiredVerticalVelocity;
			const float interpolator = ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMousePosition->Y - (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY), paddingAmountY);
			verticalModification = FMath::Lerp(minVerticalVelocityBondaryModifier, maxVerticalVelocityBondaryModifier, interpolator);
		}
	}
	desiredVerticalVelocity *= verticalModification;
	m_currentVerticalVelocity.SmoothChase(desiredVerticalVelocity, deltaTime);
	translation += m_moveUpDir * (m_currentVerticalVelocity.GetValue() * deltaTime);

	float horizontalModification = 1.0f;
	if (!IsPanningBlocked())
	{
		// LEFT
		if (cameraParameters.m_screenSpaceMousePosition->X < paddingAmountX)
		{
			desiredHorizontalVelocity = -scaledDesiredHorizontalVelocity;
			const float interpolator = 1.0f - ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMousePosition->X, paddingAmountX);
			horizontalModification = FMath::Lerp(minHorizontalVelocityBondaryModifier, maxHorizontalVelocityBondaryModifier, interpolator);
		}
		// RIGHT
		else if (cameraParameters.m_screenSpaceMousePosition->X > (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX))
		{
			desiredHorizontalVelocity = scaledDesiredHorizontalVelocity;
			const float interpolator = ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMousePosition->X - (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX), paddingAmountX);
			horizontalModification = FMath::Lerp(minHorizontalVelocityBondaryModifier, maxHorizontalVelocityBondaryModifier, interpolator);
		}
	}
	desiredHorizontalVelocity *= horizontalModification;
	m_currentHorizontalVelocity.SmoothChase(desiredHorizontalVelocity, deltaTime);
	translation += m_moveRightDir * (m_currentHorizontalVelocity.GetValue() * deltaTime);

	m_cameraPositionWithoutZoom += translation;
}

void AArgusCameraActor::UpdateCameraZoomInternal(const TOptional<FHitResult>& hitResult, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraZoom);
	UWorld* world = GetWorld();
	if (!world)
	{
		ARGUS_LOG(ArgusUnrealObjectsLog, Error, TEXT("[%s] Failed to get %s reference."), ARGUS_FUNCNAME, ARGUS_NAMEOF(UWorld));
		return;
	}

	float proposedZoomUpdateThisFrame = 0.0f;
	const FVector forwardVector = GetActorForwardVector();
	FVector proposedPosition = m_cameraPositionWithoutZoom + (m_targetZoomTranslationAmount * forwardVector);
	if (!FMath::IsNearlyZero(FMath::Sign(m_zoomInputThisFrame)))
	{
		proposedZoomUpdateThisFrame = m_zoomInputThisFrame * m_desiredZoomVelocity * deltaTime;
	}

	if (hitResult)
	{
		const float distanceUpdateThisFrame = -proposedZoomUpdateThisFrame;
		m_zoomLevelInterpolant = FMath::GetMappedRangeValueClamped(m_zoomRange, m_zeroToOne, hitResult.GetValue().Distance);

		// Camera is too close to terrain
		if ((distanceUpdateThisFrame + hitResult.GetValue().Distance) < m_minZoomDistanceToGround)
		{
			proposedPosition = hitResult.GetValue().Location + (-forwardVector * m_minZoomDistanceToGround);
			m_targetZoomTranslationAmount = FVector::Dist(proposedPosition, m_cameraPositionWithoutZoom);
		}
		// Camera is too far from terrain
		else if ((distanceUpdateThisFrame + hitResult.GetValue().Distance) > m_maxZoomDistanceToGround)
		{
			proposedPosition = hitResult.GetValue().Location + (-forwardVector * m_maxZoomDistanceToGround);
			m_targetZoomTranslationAmount = -FVector::Dist(proposedPosition, m_cameraPositionWithoutZoom);
		}
		// Camera is neither too far nor too close to terrain
		else
		{
			m_targetZoomTranslationAmount += proposedZoomUpdateThisFrame;
			proposedPosition = m_cameraPositionWithoutZoom + (m_targetZoomTranslationAmount * forwardVector);
		}
	}
	// Camera can't find any terrain
	else
	{
		m_targetZoomTranslationAmount += proposedZoomUpdateThisFrame;
		proposedPosition = m_cameraPositionWithoutZoom + (m_targetZoomTranslationAmount * forwardVector);
	}

	m_currentZoomTranslationAmount.SmoothChase(m_targetZoomTranslationAmount, deltaTime);
	SetActorLocation(m_cameraPositionWithoutZoom + (m_currentZoomTranslationAmount.GetValue() * forwardVector));
}

void AArgusCameraActor::TraceToGround(TOptional<FHitResult>& hitResult)
{
	FHitResult underlyingHitResult;
	const FVector forwardVector = GetActorForwardVector();
	const FVector proposedPosition = m_cameraPositionWithoutZoom + (m_currentZoomTranslationAmount.GetValue() * forwardVector);
	const FVector traceEndpoint = proposedPosition + (forwardVector * AArgusCameraActor::k_cameraTraceLength);
	UWorld* worldPointer = GetWorld();
	if (worldPointer->LineTraceSingleByChannel(underlyingHitResult, proposedPosition, traceEndpoint, ECC_WorldStatic))
	{
		hitResult = underlyingHitResult;
	}
	else
	{
		hitResult = NullOpt;
	}
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMath.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "Misc/Optional.h"
#include <Kismet/GameplayStatics.h>

uint8 AArgusCameraActor::s_numWidgetPanningBlockers = 0u;
FVector AArgusCameraActor::s_moveUpDir = FVector::UpVector;
FVector AArgusCameraActor::s_moveRightDir = FVector::RightVector;

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

	s_numWidgetPanningBlockers = 0u;
	s_moveUpDir = FVector::UpVector;
	s_moveRightDir = FVector::RightVector;

	m_zoomRange = TRange<float>(m_minZoomDistanceToGround, m_maxZoomDistanceToGround);
	m_zeroToOne = TRange<float>(0.0f, 1.0f);
}

void AArgusCameraActor::ForceSetCameraLocationWithoutZoom(const FVector& location, bool resetZoomSmoothing)
{
	m_cameraLocationWithoutZoom = location;
	m_currentVerticalVelocity.ResetZero();
	m_currentHorizontalVelocity.ResetZero();
	if (resetZoomSmoothing)
	{
		m_currentZoomTranslationAmount.ResetZero();
	}
	
	SetActorLocation(location);
}

void AArgusCameraActor::FocusOnArgusEntity(const ArgusEntity& entity)
{
	if (!entity)
	{
		return;
	}

	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	if (!transformComponent)
	{
		return;
	}

	FVector locationToJumpTo = transformComponent->m_location;
	locationToJumpTo.Z = m_cameraLocationWithoutZoom.Z;
	locationToJumpTo -= s_moveUpDir * m_currentOffsetRadius;

	ForceSetCameraLocationWithoutZoom(locationToJumpTo, false);
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
	m_orbitInputThisFrame = 0.0f;
}

void AArgusCameraActor::UpdateCameraOrbit(const float inputOrbitValue)
{
	m_orbitInputThisFrame += inputOrbitValue;
}

void AArgusCameraActor::UpdateCameraZoom(const float inputZoomValue)
{
	m_zoomInputThisFrame += inputZoomValue;
}

void AArgusCameraActor::UpdateCameraPanningX(const float inputValue)
{
	const float worldDelta = UGameplayStatics::GetWorldDeltaSeconds(this);
	const float scaledDesiredVerticalVelocity = FMath::Lerp(m_minimumDesiredVerticalVelocity, m_maximumDesiredVerticalVelocity, m_zoomLevelInterpolant);
	const float desiredVerticalVelocity = scaledDesiredVerticalVelocity * m_panningVelocityScale * inputValue;
	m_currentVerticalVelocity.SmoothChase(desiredVerticalVelocity, worldDelta);
	m_cameraLocationWithoutZoom += s_moveUpDir * m_currentVerticalVelocity.GetValue();
}

void AArgusCameraActor::UpdateCameraPanningY(const float inputValue)
{
	const float worldDelta = UGameplayStatics::GetWorldDeltaSeconds(this);
	const float scaledDesiredVelocity = FMath::Lerp(m_minimumDesiredHorizontalVelocity, m_maximumDesiredHorizontalVelocity, m_zoomLevelInterpolant);
	const float desiredVelocity = scaledDesiredVelocity * m_panningVelocityScale * inputValue;
	m_currentHorizontalVelocity.SmoothChase(desiredVelocity, worldDelta);
	m_cameraLocationWithoutZoom += s_moveRightDir * m_currentHorizontalVelocity.GetValue();
}

FVector AArgusCameraActor::GetCameraMoveDirection() const
{
	return m_camerMoveDirectionThisFrame;
}

void AArgusCameraActor::BeginPlay()
{
	Super::BeginPlay();
	m_currentVerticalVelocity = ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_verticalVelocitySmoothingDecayConstant);
	m_currentHorizontalVelocity = ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_horizontalVelocitySmoothingDecayConstant);
	m_currentZoomTranslationAmount = ArgusMath::ExponentialDecaySmoother<float>(0.0f, m_zoomLocationSmoothingDecayConstant);
	m_currentOrbitThetaAmount = ArgusMath::ExponentialDecaySmoother<float>(UE_PI, m_orbitThetaSmoothingDecayConstant);

	ForceSetCameraLocationWithoutZoom(GetActorLocation());

	TOptional<FHitResult> hitResult = NullOpt;
	TraceToGround(hitResult);
	if (hitResult)
	{
		const FVector2D hitLocation2D = FVector2D(hitResult.GetValue().Location);
		const FVector2D cameraLocation2D = FVector2D(m_cameraLocationWithoutZoom);
		const FVector2D unitCircleLocation = ArgusMath::ToCartesianVector2((cameraLocation2D - hitLocation2D).GetSafeNormal());
		const float arcsine = FMath::Asin(unitCircleLocation.Y);
		const float arccosine = FMath::Acos(unitCircleLocation.X);
		if (arcsine >= 0.0f)
		{
			m_currentOrbitThetaAmount.Reset(arccosine);
		}
		else if (arccosine < UE_HALF_PI)
		{
			m_currentOrbitThetaAmount.Reset(UE_TWO_PI + arcsine);
		}
		else
		{
			m_currentOrbitThetaAmount.Reset(UE_PI - arcsine);
		}
	}
	else
	{
		m_currentOrbitThetaAmount.Reset(UE_PI);
	}
	m_targetOrbitTheta = m_currentOrbitThetaAmount.GetValue();
}

void AArgusCameraActor::UpdateCameraOrbitInternal(const TOptional<FHitResult>& hitResult, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraOrbitInternal);

	if (!hitResult)
	{
		return;
	}

	// Set camera location without zoom
	const FVector2D hitResultLocation = FVector2D(hitResult.GetValue().Location);
	m_currentOffsetRadius = FVector2D::Distance(hitResultLocation, FVector2D(m_cameraLocationWithoutZoom));
	float thetaChangeThisFrame = m_orbitInputThisFrame * m_desiredOrbitVelocity * deltaTime;
	if (m_shouldInvertOrbitDirection)
	{
		thetaChangeThisFrame = -thetaChangeThisFrame;
	}
	m_targetOrbitTheta += thetaChangeThisFrame;
	m_currentOrbitThetaAmount.SmoothChase(m_targetOrbitTheta, deltaTime);

	FVector2D updatedLocation = FVector2D();
	const float currentOrbitTheta = m_currentOrbitThetaAmount.GetValue();
	updatedLocation.X = FMath::Cos(currentOrbitTheta);
	updatedLocation.Y = FMath::Sin(currentOrbitTheta);
	updatedLocation = ArgusMath::ToUnrealVector2(updatedLocation);

	s_moveUpDir = FVector(-updatedLocation, 0.0f);

	updatedLocation *= m_currentOffsetRadius;
	m_cameraLocationWithoutZoom.X = hitResultLocation.X + updatedLocation.X;
	m_cameraLocationWithoutZoom.Y = hitResultLocation.Y + updatedLocation.Y;

	// Counter rotate camera actor.
	const FVector forwardVector = hitResult.GetValue().Location - m_cameraLocationWithoutZoom;
	const FVector rightVector = -forwardVector.Cross(FVector::UpVector);
	SetActorRotation(FRotationMatrix::MakeFromXY(forwardVector, rightVector).ToQuat());

	s_moveRightDir = rightVector.GetSafeNormal();
}

void AArgusCameraActor::UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraPanning);

	if (!cameraParameters.m_screenSpaceMouseLocation || !cameraParameters.m_screenSpaceXYBounds)
	{
		return;
	}
	m_camerMoveDirectionThisFrame = FVector();
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
		float directionY = 0.0f;
		// UP
		if (cameraParameters.m_screenSpaceMouseLocation->Y < paddingAmountY)
		{
			directionY = 1.0f;
			desiredVerticalVelocity = scaledDesiredVerticalVelocity;
			const float interpolator = 1.0f - ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->Y, paddingAmountY);
			verticalModification = FMath::Lerp(minVerticalVelocityBondaryModifier, maxVerticalVelocityBondaryModifier, interpolator);
		}
		// DOWN
		else if (cameraParameters.m_screenSpaceMouseLocation->Y > (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY))
		{
			directionY = -1.0f;
			desiredVerticalVelocity = -scaledDesiredVerticalVelocity;
			const float interpolator = ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->Y - (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY), paddingAmountY);
			verticalModification = FMath::Lerp(minVerticalVelocityBondaryModifier, maxVerticalVelocityBondaryModifier, interpolator);
		}
		m_camerMoveDirectionThisFrame.Y = directionY;
	}
	desiredVerticalVelocity *= verticalModification;
	m_currentVerticalVelocity.SmoothChase(desiredVerticalVelocity, deltaTime);
	translation += s_moveUpDir * (m_currentVerticalVelocity.GetValue() * deltaTime);

	float horizontalModification = 1.0f;
	if (!IsPanningBlocked())
	{
		float directionX = 0.0f;
		// LEFT
		if (cameraParameters.m_screenSpaceMouseLocation->X < paddingAmountX)
		{
			directionX = -1.0f;
			desiredHorizontalVelocity = -scaledDesiredHorizontalVelocity;
			const float interpolator = 1.0f - ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->X, paddingAmountX);
			horizontalModification = FMath::Lerp(minHorizontalVelocityBondaryModifier, maxHorizontalVelocityBondaryModifier, interpolator);
		}
		// RIGHT
		else if (cameraParameters.m_screenSpaceMouseLocation->X > (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX))
		{
			directionX = 1.0f;
			desiredHorizontalVelocity = scaledDesiredHorizontalVelocity;
			const float interpolator = ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->X - (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX), paddingAmountX);
			horizontalModification = FMath::Lerp(minHorizontalVelocityBondaryModifier, maxHorizontalVelocityBondaryModifier, interpolator);
		}
		m_camerMoveDirectionThisFrame.X = directionX;
	}
	desiredHorizontalVelocity *= horizontalModification;
	m_currentHorizontalVelocity.SmoothChase(desiredHorizontalVelocity, deltaTime);
	translation += s_moveRightDir * (m_currentHorizontalVelocity.GetValue() * deltaTime);

	m_cameraLocationWithoutZoom += translation;
}

void AArgusCameraActor::UpdateCameraZoomInternal(const TOptional<FHitResult>& hitResult, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraZoom);

	UWorld* world = GetWorld();
	ARGUS_RETURN_ON_NULL(world, ArgusUnrealObjectsLog);

	float proposedZoomUpdateThisFrame = 0.0f;
	const FVector forwardVector = GetActorForwardVector();
	FVector proposedLocation = m_cameraLocationWithoutZoom + (m_targetZoomTranslationAmount * forwardVector);
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
			proposedLocation = hitResult.GetValue().Location + (-forwardVector * m_minZoomDistanceToGround);
			m_targetZoomTranslationAmount = FVector::Dist(proposedLocation, m_cameraLocationWithoutZoom);
		}
		// Camera is too far from terrain
		else if ((distanceUpdateThisFrame + hitResult.GetValue().Distance) > m_maxZoomDistanceToGround)
		{
			proposedLocation = hitResult.GetValue().Location + (-forwardVector * m_maxZoomDistanceToGround);
			m_targetZoomTranslationAmount = -FVector::Dist(proposedLocation, m_cameraLocationWithoutZoom);
		}
		// Camera is neither too far nor too close to terrain
		else
		{
			m_targetZoomTranslationAmount += proposedZoomUpdateThisFrame;
			proposedLocation = m_cameraLocationWithoutZoom + (m_targetZoomTranslationAmount * forwardVector);
		}
	}
	// Camera can't find any terrain
	else
	{
		m_targetZoomTranslationAmount += proposedZoomUpdateThisFrame;
		proposedLocation = m_cameraLocationWithoutZoom + (m_targetZoomTranslationAmount * forwardVector);
	}

	m_currentZoomTranslationAmount.SmoothChase(m_targetZoomTranslationAmount, deltaTime);
	SetActorLocation(m_cameraLocationWithoutZoom + (m_currentZoomTranslationAmount.GetValue() * forwardVector));
}

void AArgusCameraActor::TraceToGround(TOptional<FHitResult>& hitResult)
{
	FHitResult underlyingHitResult;
	const FVector forwardVector = GetActorForwardVector();
	const FVector proposedLocation = m_cameraLocationWithoutZoom + (m_currentZoomTranslationAmount.GetValue() * forwardVector);
	const FVector traceEndpoint = proposedLocation + (forwardVector * AArgusCameraActor::k_cameraTraceLength);
	UWorld* worldPointer = GetWorld();
	if (worldPointer->LineTraceSingleByChannel(underlyingHitResult, proposedLocation, traceEndpoint, ECC_WorldStatic))
	{
		hitResult = underlyingHitResult;
	}
	else
	{
		hitResult = NullOpt;
	}
}

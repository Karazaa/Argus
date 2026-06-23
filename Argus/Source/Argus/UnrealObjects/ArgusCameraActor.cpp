// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraActor.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMath.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "Misc/Optional.h"

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

void AArgusCameraActor::FocusOnArgusEntity(ArgusEntity entity)
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
	UpdateEntitiesInViewFrustrum();

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

void AArgusCameraActor::BeginPlay()
{
	Super::BeginPlay();
	m_currentVerticalVelocity = ArgusMath::ExponentialDecaySmoother<float>(m_verticalVelocitySmoothingDecayConstant);
	m_currentHorizontalVelocity = ArgusMath::ExponentialDecaySmoother<float>(m_horizontalVelocitySmoothingDecayConstant);
	m_currentZoomTranslationAmount = ArgusMath::ExponentialDecaySmoother<float>(m_zoomLocationSmoothingDecayConstant);
	m_currentOrbitThetaAmount = ArgusMath::ExponentialDecaySmoother<float>(UE_PI, m_orbitThetaSmoothingDecayConstant, 1.0f);

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
		if (cameraParameters.m_screenSpaceMouseLocation->Y < paddingAmountY)
		{
			desiredVerticalVelocity = scaledDesiredVerticalVelocity;
			const float interpolator = 1.0f - ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->Y, paddingAmountY);
			verticalModification = FMath::Lerp(minVerticalVelocityBondaryModifier, maxVerticalVelocityBondaryModifier, interpolator);
		}
		// DOWN
		else if (cameraParameters.m_screenSpaceMouseLocation->Y > (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY))
		{
			desiredVerticalVelocity = -scaledDesiredVerticalVelocity;
			const float interpolator = ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->Y - (cameraParameters.m_screenSpaceXYBounds->Y - paddingAmountY), paddingAmountY);
			verticalModification = FMath::Lerp(minVerticalVelocityBondaryModifier, maxVerticalVelocityBondaryModifier, interpolator);
		}
	}
	desiredVerticalVelocity *= verticalModification;
	m_currentVerticalVelocity.SmoothChase(desiredVerticalVelocity, deltaTime);
	translation += s_moveUpDir * (m_currentVerticalVelocity.GetValue() * deltaTime);

	float horizontalModification = 1.0f;
	if (!IsPanningBlocked())
	{
		// LEFT
		if (cameraParameters.m_screenSpaceMouseLocation->X < paddingAmountX)
		{
			desiredHorizontalVelocity = -scaledDesiredHorizontalVelocity;
			const float interpolator = 1.0f - ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->X, paddingAmountX);
			horizontalModification = FMath::Lerp(minHorizontalVelocityBondaryModifier, maxHorizontalVelocityBondaryModifier, interpolator);
		}
		// RIGHT
		else if (cameraParameters.m_screenSpaceMouseLocation->X > (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX))
		{
			desiredHorizontalVelocity = scaledDesiredHorizontalVelocity;
			const float interpolator = ArgusMath::SafeDivide(cameraParameters.m_screenSpaceMouseLocation->X - (cameraParameters.m_screenSpaceXYBounds->X - paddingAmountX), paddingAmountX);
			horizontalModification = FMath::Lerp(minHorizontalVelocityBondaryModifier, maxHorizontalVelocityBondaryModifier, interpolator);
		}
	}
	desiredHorizontalVelocity *= horizontalModification;
	m_currentHorizontalVelocity.SmoothChase(desiredHorizontalVelocity, deltaTime);
	translation += s_moveRightDir * (m_currentHorizontalVelocity.GetValue() * deltaTime);

	m_cameraLocationWithoutZoom += translation;
}

void AArgusCameraActor::UpdateCameraZoomInternal(const TOptional<FHitResult>& hitResult, const float deltaTime)
{
	ARGUS_TRACE(AArgusCameraActor::UpdateCameraZoomInternal);

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

void AArgusCameraActor::UpdateEntitiesInViewFrustrum()
{
	ARGUS_TRACE(AArgusCameraActor::UpdateEntitiesInViewFrustrum);

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusUnrealObjectsLog);

	ArgusIterators::IterateEntities([](ArgusEntity entity)
	{
		LODComponent* lodComponent = entity.GetComponent<LODComponent>();
		if (!lodComponent)
		{
			return;
		}

		lodComponent->PreInViewFrustrumUpdate();

		// TODO JAMES: Remove this and instead do spatial queries for within camera frustrum below.
		lodComponent->m_bIsInViewFrustrum = true;
	});

	CameraFrustrumEdges cameraFrustrumEdges;
	PopulateCameraFrustrumEdges(cameraFrustrumEdges);

	const FVector groundPlaneLocation = FVector::ZeroVector;
	const FVector flyingPlaneLocation = FVector(0.0f, 0.0f, spatialPartitioningComponent->m_flyingPlaneHeight);

	QueryEntitiesInFrustrum(groundPlaneLocation, cameraFrustrumEdges, spatialPartitioningComponent->m_argusEntityKDTree);
	QueryEntitiesInFrustrum(flyingPlaneLocation, cameraFrustrumEdges, spatialPartitioningComponent->m_flyingArgusEntityKDTree);
}

void AArgusCameraActor::PopulateCameraFrustrumEdges(CameraFrustrumEdges& frustrumEdgesToPopulate)
{
	ARGUS_TRACE(AArgusCameraActor::PopulateCameraFrustrumEdges);

	UCameraComponent* cameraComponent = GetComponentByClass<UCameraComponent>();
	ARGUS_RETURN_ON_NULL(cameraComponent, ArgusUnrealObjectsLog);

	FMinimalViewInfo viewInfo;
	cameraComponent->GetCameraView(0.0f, viewInfo);

	const float nearClipPlaneDistance = GNearClippingPlane;
	const float farClipPlaneDistance = AArgusCameraActor::k_cameraTraceLength;
	const float fieldOfView = FMath::DegreesToRadians(viewInfo.FOV);
	const float aspectRatio = viewInfo.AspectRatio;
	const FVector cameraPosition = ArgusMath::ToCartesianVector(GetActorLocation());
	const FVector cameraViewDirection = ArgusMath::ToCartesianVector(GetActorForwardVector());
	const FVector cameraUpDirection = ArgusMath::ToCartesianVector(GetActorUpVector());
	const FVector cameraRightDirection = ArgusMath::ToCartesianVector(GetActorRightVector());

	const float tangentFOV = 2.0f * FMath::Tan(fieldOfView * 0.5f);
	const float nearPlaneHeight = tangentFOV * nearClipPlaneDistance;
	const float nearPlaneWidth = nearPlaneHeight * aspectRatio;
	const float farPlaneHeight = tangentFOV * farClipPlaneDistance;
	const float farPlaneWidth = farPlaneHeight * aspectRatio;

	const FVector centerOfNearPlane = cameraPosition + (cameraViewDirection * nearClipPlaneDistance);
	const FVector centerOfFarPlane = cameraPosition + (cameraViewDirection * farClipPlaneDistance);

	const FVector nearUpOffset = cameraUpDirection * (nearPlaneHeight * 0.5f);
	const FVector nearRightOffset = cameraRightDirection * (nearPlaneWidth * 0.5f);
	const FVector farUpOffset = cameraUpDirection * (farPlaneHeight * 0.5f);
	const FVector farRightOffset = cameraRightDirection * (farPlaneWidth * 0.5f);

	frustrumEdgesToPopulate.m_topLeftLocation = ArgusMath::ToUnrealVector(centerOfNearPlane + nearUpOffset - nearRightOffset);
	frustrumEdgesToPopulate.m_topRightLocation = ArgusMath::ToUnrealVector(centerOfNearPlane + nearUpOffset + nearRightOffset);
	frustrumEdgesToPopulate.m_bottomLeftLocation = ArgusMath::ToUnrealVector(centerOfNearPlane - nearUpOffset - nearRightOffset);
	frustrumEdgesToPopulate.m_bottomRightLocation = ArgusMath::ToUnrealVector(centerOfNearPlane - nearUpOffset + nearRightOffset);
	frustrumEdgesToPopulate.m_topLeftDirection = (ArgusMath::ToUnrealVector(centerOfFarPlane + farUpOffset - farRightOffset) - frustrumEdgesToPopulate.m_topLeftLocation).GetSafeNormal();
	frustrumEdgesToPopulate.m_topRightDirection = (ArgusMath::ToUnrealVector(centerOfFarPlane + farUpOffset + farRightOffset) - frustrumEdgesToPopulate.m_topRightLocation).GetSafeNormal();
	frustrumEdgesToPopulate.m_bottomLeftDirection = (ArgusMath::ToUnrealVector(centerOfFarPlane - farUpOffset - farRightOffset) - frustrumEdgesToPopulate.m_bottomLeftLocation).GetSafeNormal();
	frustrumEdgesToPopulate.m_bottomRightDirection = (ArgusMath::ToUnrealVector(centerOfFarPlane - farUpOffset + farRightOffset) - frustrumEdgesToPopulate.m_bottomRightLocation).GetSafeNormal();
}

void AArgusCameraActor::QueryEntitiesInFrustrum(const FVector& planeLocation, const CameraFrustrumEdges& cameraFrustrumEdges, const ArgusEntityKDTree& entityKDTree) const
{
	ARGUS_TRACE(AArgusCameraActor::QueryEntitiesInFrustrum);

	const FVector planeNormal = FVector::UpVector;
	FVector topLeftIntersection, topRightIntersection, bottomLeftIntersection, bottomRightIntersection;
	if (!ArgusMath::GetRayToPlaneIntersection(cameraFrustrumEdges.m_topLeftLocation, cameraFrustrumEdges.m_topLeftDirection, planeLocation, planeNormal, topLeftIntersection) ||
		!ArgusMath::GetRayToPlaneIntersection(cameraFrustrumEdges.m_topRightLocation, cameraFrustrumEdges.m_topRightDirection, planeLocation, planeNormal, topRightIntersection) ||
		!ArgusMath::GetRayToPlaneIntersection(cameraFrustrumEdges.m_bottomLeftLocation, cameraFrustrumEdges.m_bottomLeftDirection, planeLocation, planeNormal, bottomLeftIntersection) ||
		!ArgusMath::GetRayToPlaneIntersection(cameraFrustrumEdges.m_bottomRightLocation, cameraFrustrumEdges.m_bottomRightDirection, planeLocation, planeNormal, bottomRightIntersection))
	{
		return;
	}

	// TODO JAMES: Make polygon and query the KDTree.
}

void AArgusCameraActor::TraceToGround(TOptional<FHitResult>& hitResult)
{
	ARGUS_TRACE(AArgusCameraActor::TraceToGround);

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

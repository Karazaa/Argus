// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraFunctionalTest.h"
#include "ArgusCameraActor.h"
#include "ArgusMacros.h"

const FVector2D k_screenBounds		= FVector2D(100.0f, 100.0f);
const FVector2D k_neutralScreenPos	= FVector2D(50.0f, 50.0f);
const FVector2D k_leftScreenPos		= FVector2D(1.0f, 50.0f);
const FVector2D k_rightScreenPos	= FVector2D(99.0f, 50.0f);
const FVector2D k_upScreenPos		= FVector2D(50.0f, 1.0f);
const FVector2D k_downScreenPos		= FVector2D(50.0f, 99.0f);

const float k_zoomPositionDifferenceSquaredTolerance = 5.0f;

AArgusCameraFunctionalTest::AArgusCameraFunctionalTest(const FObjectInitializer& ObjectInitializer)
{
	TestLabel = k_testName;
}

bool AArgusCameraFunctionalTest::DidArgusFunctionalTestFail()
{
	if (!m_argusCameraActor.IsValid())
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusCameraActor)
		);
		return true;
	}
	return false;
}

void AArgusCameraFunctionalTest::StartNextTestStep()
{
	if (DidArgusFunctionalTestFail())
	{
		return;
	}

	if (m_testStepIndex == 0)
	{
		m_testStartingLocation = m_argusCameraActor->GetActorLocation();
	}

	m_argusCameraActor->ForceSetCameraLocationWithoutZoom(m_testStartingLocation);

	switch (m_testStepIndex)
	{
		case 0u:
			StartPanLeftTestStep();
			break;
		case 1u:
			StartPanRightTestStep();
			break;
		case 2u:
			StartPanUpTestStep();
			break;
		case 3u:
			StartPanDownTestStep();
			break;
		case 4u:
			StartZoomInTestStep();
			break;
		case 5u:
			StartZoomOutTestStep();
			break;
		default:
			break;
	}
}

bool AArgusCameraFunctionalTest::DidCurrentTestStepSucceed()
{
	if (DidArgusFunctionalTestFail())
	{
		return false;
	}

	const FVector locationDifference = m_argusCameraActor->GetActorLocation() - m_testStartingLocation;

	switch (m_testStepIndex)
	{
		case 0u:
			return DidPanLeftTestStepSucceed(locationDifference);
		case 1u:
			return DidPanRightTestStepSucceed(locationDifference);
		case 2u:
			return DidPanUpTestStepSucceed(locationDifference);
		case 3u:
			return DidPanDownTestStepSucceed(locationDifference);
		case 4u:
			return DidZoomInTestStepSucceed();
		case 5u:
			return DidZoomOutTestStepSucceed();
		default:
			break;
	}
	return false;
}

void AArgusCameraFunctionalTest::StartPanLeftTestStep()
{
	StartStep(TEXT("Pan camera left"));
}

void AArgusCameraFunctionalTest::StartPanRightTestStep()
{
	StartStep(TEXT("Pan camera right"));
}

void AArgusCameraFunctionalTest::StartPanUpTestStep()
{
	StartStep(TEXT("Pan camera up"));
}

void AArgusCameraFunctionalTest::StartPanDownTestStep()
{
	StartStep(TEXT("Pan camera down"));
}

void AArgusCameraFunctionalTest::StartZoomInTestStep()
{
	StartStep(TEXT("Zoom in"));
}

void AArgusCameraFunctionalTest::StartZoomOutTestStep()
{
	StartStep(TEXT("Zoom out"));
}

bool AArgusCameraFunctionalTest::DidPanLeftTestStepSucceed(const FVector& locationDifference)
{
	if (FVector::Coincident(-m_argusCameraActor->GetPanRightVector(), locationDifference.GetSafeNormal()) && locationDifference.SquaredLength() > FMath::Square(m_targetPanLeftRightQuantity))
	{
		return true;
	}

	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = k_screenBounds;
	cameraParameters.m_screenSpaceMouseLocation = k_leftScreenPos;
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	return false;
}

bool AArgusCameraFunctionalTest::DidPanRightTestStepSucceed(const FVector& locationDifference)
{
	if (FVector::Coincident(m_argusCameraActor->GetPanRightVector(), locationDifference.GetSafeNormal()) && locationDifference.SquaredLength() > FMath::Square(m_targetPanLeftRightQuantity))
	{
		return true;
	}

	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = k_screenBounds;
	cameraParameters.m_screenSpaceMouseLocation = k_rightScreenPos;
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	return false;
}

bool AArgusCameraFunctionalTest::DidPanUpTestStepSucceed(const FVector& locationDifference)
{
	if (FVector::Coincident(AArgusCameraActor::GetPanUpVector(), locationDifference.GetSafeNormal()) && locationDifference.SquaredLength() > FMath::Square(m_targetPanUpDownQuantity))
	{
		return true;
	}

	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = k_screenBounds;
	cameraParameters.m_screenSpaceMouseLocation = k_upScreenPos;
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	return false;
}

bool AArgusCameraFunctionalTest::DidPanDownTestStepSucceed(const FVector& locationDifference)
{
	if (FVector::Coincident(-AArgusCameraActor::GetPanUpVector(), locationDifference.GetSafeNormal()) && locationDifference.SquaredLength() > FMath::Square(m_targetPanUpDownQuantity))
	{
		return true;
	}

	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = k_screenBounds;
	cameraParameters.m_screenSpaceMouseLocation = k_downScreenPos;
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	return false;
}

bool AArgusCameraFunctionalTest::DidZoomInTestStepSucceed()
{
	const FVector initialZoomTargetTranslation = m_argusCameraActor->GetZoomTargetTranslation();

	m_argusCameraActor->UpdateCameraZoom(1.0f);
	return UpdateCameraZoomAndCheckAtBoundaries(initialZoomTargetTranslation) && initialZoomTargetTranslation.Z < 0.0f;
}

bool AArgusCameraFunctionalTest::DidZoomOutTestStepSucceed()
{
	const FVector initialZoomTargetTranslation = m_argusCameraActor->GetZoomTargetTranslation();

	m_argusCameraActor->UpdateCameraZoom(-1.0f);
	return UpdateCameraZoomAndCheckAtBoundaries(initialZoomTargetTranslation) && initialZoomTargetTranslation.Z > 0.0f;
}

bool AArgusCameraFunctionalTest::UpdateCameraZoomAndCheckAtBoundaries(const FVector& initialZoomTargetTranslation)
{
	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = k_screenBounds;
	cameraParameters.m_screenSpaceMouseLocation = k_neutralScreenPos;
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	const FVector postUpdateZoomTargetTranslation = m_argusCameraActor->GetZoomTargetTranslation();
	const FVector cameraPositionWithoutZoom = m_argusCameraActor->GetCameraLocationWithoutZoom();

	const float translationMagnitudeSquaredDifference = FMath::Abs(initialZoomTargetTranslation.SquaredLength() - postUpdateZoomTargetTranslation.SquaredLength());
	const float locationMagnitudeSquaredDifference = FMath::Abs((postUpdateZoomTargetTranslation + cameraPositionWithoutZoom).SquaredLength() - m_argusCameraActor->GetActorLocation().SquaredLength());

	return translationMagnitudeSquaredDifference < k_zoomPositionDifferenceSquaredTolerance && locationMagnitudeSquaredDifference < k_zoomPositionDifferenceSquaredTolerance;
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCameraFunctionalTest.h"
#include "ArgusCameraActor.h"
#include "ArgusMacros.h"

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

	m_argusCameraActor->ForceSetCameraPositionWithoutZoom(m_testStartingLocation);

	switch (m_testStepIndex)
	{
		case 0:
			StartPanLeftTestStep();
			break;
		case 1:
			StartPanRightTestStep();
			break;
		case 2:
			StartPanUpTestStep();
			break;
		case 3:
			StartPanDownTestStep();
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
		case 0:
			return DidPanLeftTestStepSucceed(locationDifference);
		case 1:
			return DidPanRightTestStepSucceed(locationDifference);
		case 2:
			return DidPanUpTestStepSucceed(locationDifference);
		case 3:
			return DidPanDownTestStepSucceed(locationDifference);
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

bool AArgusCameraFunctionalTest::DidPanLeftTestStepSucceed(const FVector& locationDifference)
{
	if (FVector::Coincident(-m_argusCameraActor->GetPanRightVector(), locationDifference.GetSafeNormal()) && locationDifference.SquaredLength() > FMath::Square(m_targetPanLeftRightQuantity))
	{
		return true;
	}

	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = FVector2D(100.0f, 100.0f);
	cameraParameters.m_screenSpaceMousePosition = FVector2D(1.0f, 50.0f);
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
	cameraParameters.m_screenSpaceXYBounds = FVector2D(100.0f, 100.0f);
	cameraParameters.m_screenSpaceMousePosition = FVector2D(99.0f, 50.0f);
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	return false;
}

bool AArgusCameraFunctionalTest::DidPanUpTestStepSucceed(const FVector& locationDifference)
{
	if (FVector::Coincident(m_argusCameraActor->GetPanUpVector(), locationDifference.GetSafeNormal()) && locationDifference.SquaredLength() > FMath::Square(m_targetPanUpDownQuantity))
	{
		return true;
	}

	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = FVector2D(100.0f, 100.0f);
	cameraParameters.m_screenSpaceMousePosition = FVector2D(50.0f, 1.0f);
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	return false;
}

bool AArgusCameraFunctionalTest::DidPanDownTestStepSucceed(const FVector& locationDifference)
{
	if (FVector::Coincident(-m_argusCameraActor->GetPanUpVector(), locationDifference.GetSafeNormal()) && locationDifference.SquaredLength() > FMath::Square(m_targetPanUpDownQuantity))
	{
		return true;
	}

	AArgusCameraActor::UpdateCameraPanningParameters cameraParameters;
	cameraParameters.m_screenSpaceXYBounds = FVector2D(100.0f, 100.0f);
	cameraParameters.m_screenSpaceMousePosition = FVector2D(50.0f, 99.0f);
	m_argusCameraActor->UpdateCamera(cameraParameters, m_currentDeltaSeconds);

	return false;
}

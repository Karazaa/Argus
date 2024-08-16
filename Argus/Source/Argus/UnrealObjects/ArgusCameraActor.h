// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArgusCameraActor.generated.h"

UCLASS()
class AArgusCameraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AArgusCameraActor();

	struct UpdateCameraPanningParameters
	{
		TOptional<FVector2D> m_screenSpaceMousePosition;
		TOptional<FVector2D> m_screenSpaceXYBounds;
	};
	void UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, float deltaTime);
	void UpdateCameraZoom(float inputZoomValue);

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float m_screenMovePaddingProportion = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float m_cameraMoveVelocity = 500.0f;

private:
	// TODO JAMES: Do these for real.
	FVector m_moveUpDir = FVector::ForwardVector;
	FVector m_moveRightDir = FVector::RightVector;
};

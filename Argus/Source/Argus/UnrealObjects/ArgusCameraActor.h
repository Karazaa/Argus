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

	void UpdateCamera(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime);
	void UpdateCameraZoom(const float inputZoomValue);

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_verticalScreenMovePaddingProportion = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_horizontalScreenMovePaddingProportion = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_desiredVerticalVelocity = 500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_verticalAcceleration = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_desiredHorizontalVelocity = 500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_horizontalAcceleration = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_desiredZoomVelocity = 500.0f;

	virtual void BeginPlay() override;

private:
	void UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime);
	void UpdateCameraZoomInternal(const float deltaTime);

	float m_currentVerticalVelocity = 0.0f;
	float m_currentHorizontalVelocity = 0.0f;
	float m_zoomInputThisFrame = 0.0f;
	FVector m_zoomTargetLocation = FVector::ZeroVector;


	// TODO JAMES: Do these for real.
	FVector m_moveUpDir		= FVector::ForwardVector;
	FVector m_moveRightDir	= FVector::RightVector;
};

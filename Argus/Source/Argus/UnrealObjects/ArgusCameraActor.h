// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMath.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArgusCameraActor.generated.h"

UCLASS()
class AArgusCameraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	static constexpr float k_cameraTraceLength = 10000.0f;

	AArgusCameraActor();

	struct UpdateCameraPanningParameters
	{
		TOptional<FVector2D> m_screenSpaceMousePosition;
		TOptional<FVector2D> m_screenSpaceXYBounds;
	};

	void ForceSetCameraPositionWithoutZoom(const FVector& position);
	void UpdateCamera(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime);
	void UpdateCameraZoom(const float inputZoomValue);

	const FVector& GetPanUpVector() { return m_moveUpDir; }
	const FVector& GetPanRightVector() { return m_moveRightDir; }
	const FVector& GetZoomTargetTranslation() { return m_zoomTargetTranslation; }
	const FVector& GetCameraPositionWithoutZoom() { return m_cameraPositionWithoutZoom; }

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_verticalScreenMovePaddingProportion = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_horizontalScreenMovePaddingProportion = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_desiredVerticalVelocity = 500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning", Meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float m_verticalVelocitySmoothingDecayConstant = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_desiredHorizontalVelocity = 500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning", Meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float m_horizontalVelocitySmoothingDecayConstant = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_desiredZoomVelocity = 500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_zoomAcceleration = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_minZoomDistanceToGround = 100.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_maxZoomDistanceToGround = 10000.0f;

	virtual void BeginPlay() override;

private:
	void UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime);
	void UpdateCameraZoomInternal(const float deltaTime);

	ArgusMath::ExponentialDecaySmoother<float> m_currentVerticalVelocity;
	ArgusMath::ExponentialDecaySmoother<float> m_currentHorizontalVelocity;
	// TODO JAMES FVector Smoother for current location.

	float m_zoomInputThisFrame = 0.0f;
	FVector m_zoomTargetTranslation = FVector::ZeroVector;
	FVector m_cameraPositionWithoutZoom = FVector::ZeroVector;


	// TODO JAMES: Do these for real.
	FVector m_moveUpDir		= FVector::ForwardVector;
	FVector m_moveRightDir	= FVector::RightVector;
};

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
	static void IncrementPanningBlockers();
	static void DecrementPanningBlockers();
	static FVector& GetPanUpVector() { return s_moveUpDir; }
	static FVector& GetPanRightVector() { return s_moveRightDir; }

	AArgusCameraActor();

	struct UpdateCameraPanningParameters
	{
		TOptional<FVector2D> m_screenSpaceMouseLocation;
		TOptional<FVector2D> m_screenSpaceXYBounds;
	};

	void ForceSetCameraLocationWithoutZoom(const FVector& location);
	void UpdateCamera(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime);
	void UpdateCameraOrbit(const float inputOrbitValue);
	void UpdateCameraZoom(const float inputZoomValue);

	const FVector GetZoomTargetTranslation() { return m_currentZoomTranslationAmount.GetValue() * GetActorForwardVector(); }
	const FVector& GetCameraLocationWithoutZoom() { return m_cameraLocationWithoutZoom; }

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_verticalScreenMovePaddingProportion = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_horizontalScreenMovePaddingProportion = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_minimumDesiredVerticalVelocity = 300.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_maximumDesiredVerticalVelocity = 600.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning", Meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float m_verticalVelocityBoundaryModifier = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning", Meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float m_verticalVelocitySmoothingDecayConstant = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_minimumDesiredHorizontalVelocity = 300.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning")
	float m_maximumDesiredHorizontalVelocity = 600.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning", Meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float m_horizontalVelocityBoundaryModifier = 0.2f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Panning", Meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float m_horizontalVelocitySmoothingDecayConstant = 5.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_desiredZoomVelocity = 500.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_zoomLocationSmoothingDecayConstant = 10.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_minZoomDistanceToGround = 100.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Zoom")
	float m_maxZoomDistanceToGround = 10000.0f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Orbit")
	bool m_shouldInvertOrbitDirection = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Orbit")
	float m_desiredOrbitVelocity = 0.1f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Orbit")
	float m_orbitThetaSmoothingDecayConstant = 5.0f;

	virtual void BeginPlay() override;

private:
	static uint8 s_numWidgetPanningBlockers;
	static FVector s_moveUpDir;
	static FVector s_moveRightDir;

	bool IsPanningBlocked() const { return s_numWidgetPanningBlockers != 0u || m_orbitInputThisFrame != 0.0f; }

	void UpdateCameraOrbitInternal(const TOptional<FHitResult>& hitResult, const float deltaTime);
	void UpdateCameraPanning(const UpdateCameraPanningParameters& cameraParameters, const float deltaTime);
	void UpdateCameraZoomInternal(const TOptional<FHitResult>& hitResult, const float deltaTime);

	void TraceToGround(TOptional<FHitResult>& hitResult);

	ArgusMath::ExponentialDecaySmoother<float>		m_currentVerticalVelocity;
	ArgusMath::ExponentialDecaySmoother<float>		m_currentHorizontalVelocity;
	ArgusMath::ExponentialDecaySmoother<float>		m_currentZoomTranslationAmount;
	ArgusMath::ExponentialDecaySmoother<float>		m_currentOrbitThetaAmount;

	float m_orbitInputThisFrame = 0.0f;
	float m_targetOrbitTheta = 0.0f;

	float m_zoomInputThisFrame = 0.0f;
	float m_zoomLevelInterpolant = 0.5f;
	float m_targetZoomTranslationAmount = 0.0f;
	TRange<float> m_zoomRange;
	TRange<float> m_zeroToOne;

	FVector m_cameraLocationWithoutZoom = FVector::ZeroVector;
};

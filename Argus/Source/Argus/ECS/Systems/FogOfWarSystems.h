// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include <immintrin.h>

class ArgusEntity;

struct FogOfWarComponent;
struct FogOfWarSystemsArgs;
struct ObstacleIndicies;
struct InputInterfaceComponent;
struct SpatialPartitioningComponent;

class FogOfWarSystems
{
public:
	static void InitializeSystems();
	static void RunSystems();
	static void RunThreadSystems(float deltaTime);

private:
	struct FogOfWarOffsets
	{
		uint32 m_leftOffset = 0u;
		uint32 m_rightOffset = 0u;
		uint32 m_topOffset = 0u;
		uint32 m_bottomOffset = 0u;

		uint32 m_circleX = 0u;
		uint32 m_circleY = 0u;
		uint32 m_innerCircleX = 0u;
	};

	struct CircleQuadrant
	{
		uint32 m_yValue = 0u;
		uint32 m_xStartValue = 0u;
		uint32 m_xEndValue = 0u;
		uint32 m_centerColumnIndex = 0u;
	};

	struct QuadrantObstacleTraces
	{
		FVector2D m_previousLeft = FVector2D::ZeroVector;
		FVector2D m_previousRight = FVector2D::ZeroVector;

		QuadrantObstacleTraces(FVector2D initialPosition) : m_previousLeft(initialPosition), m_previousRight(initialPosition) {}
		QuadrantObstacleTraces() {}
	};

	struct CircleOctantExpansion
	{
		uint32 m_topY = 0u;
		uint32 m_topStartX = 0u;
		uint32 m_topEndX =  0u;

		uint32 m_bottomY = 0u;
		uint32 m_bottomStartX = 0u;
		uint32 m_bottomEndX = 0u;

		uint32 m_midUpY = 0u;
		uint32 m_midUpStartX = 0u;
		uint32 m_midUpEndX = 0u;

		uint32 m_midDownY = 0u;
		uint32 m_midDownStartX = 0u;
		uint32 m_midDownEndX = 0u;

		uint32 m_centerColumnTopIndex = 0u;
		uint32 m_centerColumnMidUpIndex = 0u;
		uint32 m_centerColumnMidDownIndex = 0u;
		uint32 m_centerColumnBottomIndex = 0u;
	};

	struct OctantTraces
	{
		FVector2D m_previousTopLeft = FVector2D::ZeroVector;
		FVector2D m_previousTopRight = FVector2D::ZeroVector;

		FVector2D m_previousBottomLeft = FVector2D::ZeroVector;
		FVector2D m_previousBottomRight = FVector2D::ZeroVector;

		FVector2D m_previousMidUpLeft = FVector2D::ZeroVector;
		FVector2D m_previousMidUpRight = FVector2D::ZeroVector;

		FVector2D m_previousMidDownLeft = FVector2D::ZeroVector;
		FVector2D m_previousMidDownRight = FVector2D::ZeroVector;

		OctantTraces(FVector2D initialPosition) :	m_previousTopLeft(initialPosition), m_previousTopRight(initialPosition),
													m_previousBottomLeft(initialPosition), m_previousBottomRight(initialPosition),
													m_previousMidUpLeft(initialPosition), m_previousMidUpRight(initialPosition),
													m_previousMidDownLeft(initialPosition), m_previousMidDownRight(initialPosition)
		{}
	};

	static void InitializeGaussianFilter(FogOfWarComponent* fogOfWarComponent);
	static void ClearActivelyRevealedPixels(FogOfWarComponent* fogOfWarComponent);
	static void ClearActivelyRevealedPixelsForRange(FogOfWarComponent* fogOfWarComponent, int32 fromInclusive, int32 toExclusive);
	static void SetRevealedStatePerEntity(FogOfWarComponent* fogOfWarComponent);
	static void ApplyExponentialDecaySmoothing(FogOfWarComponent* fogOfWarComponent, float deltaTime);
	static void ApplyExponentialDecaySmoothingForRange(FogOfWarComponent* fogOfWarComponent, float deltaTime, const __m256& exponentialDecayCoefficient, int32 fromInclusive, int32 toExclusive);
	static void PopulateOffsetsForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, FogOfWarOffsets& outOffsets);
	static void PopulateOctantExpansionForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets, CircleOctantExpansion& outCircleOctantExpansion);
	static void RevealPixelAlphaForEntity(FogOfWarComponent* fogOfWarComponent, uint16 entityId);
	static void RasterizeCircleOfRadius(FogOfWarComponent* fogOfWarComponent, uint32 radius, FogOfWarOffsets& offsets, bool accountForTriangleRasterization, TFunction<void (FogOfWarOffsets& offsets)> perOctantPixelFunction);
	static void RasterizeTriangleForReveal(FogOfWarComponent* fogOfWarComponent, const FVector2D& point0, const FVector2D& point1, const FVector2D& point2);
	static void FillFlatBottomTriangle(FogOfWarComponent* fogOfWarComponent, const TPair<int32, int32>& point0, const TPair<int32, int32>& point1, const TPair<int32, int32>& point2);
	static void FillFlatTopTriangle(FogOfWarComponent* fogOfWarComponent, const TPair<int32, int32>& point0, const TPair<int32, int32>& point1, const TPair<int32, int32>& point2);
	static void SetAlphaForPixelRange(FogOfWarComponent* fogOfWarComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive);
	static void RevealPixelRangeWithObstacles(FogOfWarComponent* fogOfWarComponent, const SpatialPartitioningComponent* spatialPartitioningComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, const TArray<ObstacleIndicies>& obstacleIndicies, const FVector2D& cartesianEntityLocation, FVector2D& prevFrom, FVector2D& prevTo);
	static void SetAlphaForCircleQuadrant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const CircleQuadrant& quadrant, const TArray<ObstacleIndicies>& obstacleIndicies, QuadrantObstacleTraces& quadrantTraces);
	static void UpdateTexture();
	static void UpdateGaussianWeightsTexture();
	static void UpdateDynamicMaterialInstance();

	static bool GetPixelCoordsFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector2D& worldSpaceLocation, TPair<int32, int32>& ouputPair);
	static uint32 GetPixelNumberFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector& worldSpaceLocation);
	static FVector2D GetWorldSpaceLocationFromPixelNumber(FogOfWarComponent* fogOfWarComponent, uint32 pixelNumber);
	static uint32 GetPixelRadiusFromWorldSpaceRadius(FogOfWarComponent* fogOfWarComponent, float radius);
	static void ClampVectorToWorldBounds(FVector2D& vector);
};

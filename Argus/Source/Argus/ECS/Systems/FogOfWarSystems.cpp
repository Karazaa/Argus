// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FogOfWarSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "SystemArgumentDefinitions/FogOfWarSystemsArgs.h"

#include <immintrin.h>

void FogOfWarSystems::InitializeSystems()
{
	ARGUS_TRACE(FogOfWarSystems::InitializeSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	fogOfWarComponent->m_fogOfWarTexture = UTexture2D::CreateTransient(fogOfWarComponent->m_textureSize, fogOfWarComponent->m_textureSize, PF_A8);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_fogOfWarTexture, ArgusECSLog);

	fogOfWarComponent->m_gaussianWeightsTexture = UTexture2D::CreateTransient(fogOfWarComponent->m_gaussianDimension, fogOfWarComponent->m_gaussianDimension, PF_R32_FLOAT);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_gaussianWeightsTexture, ArgusECSLog);

	fogOfWarComponent->m_fogOfWarTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	fogOfWarComponent->m_fogOfWarTexture->SRGB = 0;
	fogOfWarComponent->m_fogOfWarTexture->Filter = TextureFilter::TF_Nearest;
	fogOfWarComponent->m_fogOfWarTexture->AddToRoot();
	fogOfWarComponent->m_fogOfWarTexture->UpdateResource();
	fogOfWarComponent->m_textureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, fogOfWarComponent->m_textureSize, fogOfWarComponent->m_textureSize);

	fogOfWarComponent->m_gaussianWeightsTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	fogOfWarComponent->m_gaussianWeightsTexture->SRGB = 0;
	fogOfWarComponent->m_gaussianWeightsTexture->Filter = TextureFilter::TF_Nearest;
	fogOfWarComponent->m_gaussianWeightsTexture->AddToRoot();
	fogOfWarComponent->m_gaussianWeightsTexture->UpdateResource();

	fogOfWarComponent->m_textureData.Init(255u, fogOfWarComponent->GetTotalPixels());
	if (fogOfWarComponent->m_shouldUseSmoothing)
	{
		fogOfWarComponent->m_smoothedTextureData.Init(255u, fogOfWarComponent->GetTotalPixels());
		fogOfWarComponent->m_intermediarySmoothingData.Init(255.0f, fogOfWarComponent->GetTotalPixels());
	}

	InitializeGaussianFilter(fogOfWarComponent);
	UpdateDynamicMaterialInstance();
}

void FogOfWarSystems::RunThreadSystems(float deltaTime)
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Iterate over all entities and carve out a circle of pixels (activelyRevealed) based on sight radius for entities that are on the local player team (or allies).
	SetRevealedStatePerEntity(fogOfWarComponent);

	// Take our result target state and use exponential decay smoothing to get a final state.
	if (fogOfWarComponent->m_shouldUseSmoothing)
	{
		ApplyExponentialDecaySmoothing(fogOfWarComponent, deltaTime);
	}
}

void FogOfWarSystems::RunSystems()
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	UpdateTexture();
}

void FogOfWarSystems::InitializeGaussianFilter(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	fogOfWarComponent->m_gaussianFilter.SetNumZeroed(fogOfWarComponent->m_gaussianDimension * fogOfWarComponent->m_gaussianDimension);

	TArray<float> oneDimensionalFilter;
	oneDimensionalFilter.SetNumZeroed(fogOfWarComponent->m_gaussianDimension);

	const float radius = static_cast<float>(fogOfWarComponent->m_gaussianDimension / 2);
	const float radiusSquaredReciprocal = 1.0f / (2.0f * FMath::Square(radius));
	const float squareRootPiReciprocal = 1.0f / (FMath::Sqrt(UE_TWO_PI) * radius);

	float shiftedRadius = -radius;
	float sum = 0.0f;
	for (uint8 i = 0; i < fogOfWarComponent->m_gaussianDimension; ++i)
	{
		const float squareShiftedRadius = FMath::Square(shiftedRadius);
		oneDimensionalFilter[i] = squareRootPiReciprocal * FMath::Exp(-squareShiftedRadius * radiusSquaredReciprocal);
		sum += oneDimensionalFilter[i];
		shiftedRadius += 1.0f;
	}

	if (FMath::IsNearlyZero(sum))
	{
		return;
	}

	for (uint8 i = 0; i < fogOfWarComponent->m_gaussianDimension; ++i)
	{
		oneDimensionalFilter[i] /= sum;
	}

	for (uint8 i = 0; i < fogOfWarComponent->m_gaussianDimension; ++i)
	{
		for (uint8 j = 0; j < fogOfWarComponent->m_gaussianDimension; ++j)
		{
			uint8 index = i + (j * fogOfWarComponent->m_gaussianDimension);
			fogOfWarComponent->m_gaussianFilter[index] = oneDimensionalFilter[i] * oneDimensionalFilter[j];
		}
	}

	UpdateGaussianWeightsTexture();
}

void FogOfWarSystems::SetRevealedStatePerEntity(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_TRACE(FogOfWarSystems::SetRevealedPixels);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	// Clear dead entity pixels.
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		if (entity.IsAlive())
		{
			continue;
		}

		FogOfWarLocationComponent* fogOfWarLocationComponent = entity.GetComponent<FogOfWarLocationComponent>();
		if (!fogOfWarLocationComponent)
		{
			continue;
		}

		fogOfWarLocationComponent->m_clearedThisFrame = fogOfWarLocationComponent->m_fogOfWarPixel == MAX_uint32 ? false : true;
	}

	// Set actively revealed pixels to revealed once.
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(entity))
		{
			continue;
		}

		if (!entity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam))
		{
			continue;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);

		const uint32 centerPixel = GetPixelNumberFromWorldSpaceLocation(fogOfWarComponent, components.m_transformComponent->m_location);
		const bool newCenterPixel = centerPixel != components.m_fogOfWarLocationComponent->m_fogOfWarPixel;
		if (newCenterPixel && entity.IsAlive() && components.m_fogOfWarLocationComponent->m_fogOfWarPixel != MAX_uint32)
		{
			RevealPixelAlphaForEntity(fogOfWarComponent, components, offsets, false);
			components.m_fogOfWarLocationComponent->m_updatedPixelThisFrame = true;
		}
		else if (components.m_fogOfWarLocationComponent->m_clearedThisFrame)
		{
			RevealPixelAlphaForEntity(fogOfWarComponent, components, offsets, false);
			components.m_fogOfWarLocationComponent->m_fogOfWarPixel = MAX_uint32;
			components.m_fogOfWarLocationComponent->m_updatedPixelThisFrame = false;
		}
		else
		{
			components.m_fogOfWarLocationComponent->m_updatedPixelThisFrame = false;
		}

		if (entity.IsAlive())
		{
			components.m_fogOfWarLocationComponent->m_fogOfWarPixel = centerPixel;
		}
	}

	// Calculate new actively revealed pixels.
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(entity))
		{
			continue;
		}

		if (!components.m_fogOfWarLocationComponent->m_updatedPixelThisFrame)
		{
			UpdateDoesEntityNeedToUpdateActivelyRevealed(components, inputInterfaceComponent);
			if (!components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame)
			{
				continue;
			}
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		RevealPixelAlphaForEntity(fogOfWarComponent, components, offsets, true);
	}
}

void FogOfWarSystems::ApplyExponentialDecaySmoothing(FogOfWarComponent* fogOfWarComponent, float deltaTime)
{
	ARGUS_TRACE(FogOfWarSystems::ApplyExponentialDecaySmoothing);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (FMath::IsNearlyZero(deltaTime))
	{
		return;
	}

	static constexpr int32 topIterationSize = 128;
	static constexpr int32 midIterationSize = 32;
	static constexpr int32 smallIterationSize = 8;
	const uint8* sourceArray = fogOfWarComponent->m_textureData.GetData();
	const __m256 exponentialDecayCoefficient = _mm256_set1_ps(FMath::Exp(-fogOfWarComponent->m_smoothingDecayConstant * deltaTime));

	// value = targetValue + ((value - targetValue) * FMath::Exp(-decayConstant * deltaTime));
	for (int32 i = 0; i < static_cast<int32>(fogOfWarComponent->GetTotalPixels()); i += topIterationSize)
	{
		if (memcmp(&sourceArray[i], &fogOfWarComponent->m_smoothedTextureData[i], topIterationSize) == 0)
		{
			continue;
		}

		for (int32 j = i; j < i + topIterationSize; j += midIterationSize)
		{
			if (memcmp(&sourceArray[j], &fogOfWarComponent->m_smoothedTextureData[j], midIterationSize) == 0)
			{
				continue;
			}

			for (int32 k = j; k < j + midIterationSize; k += smallIterationSize)
			{
				if (memcmp(&sourceArray[k], &fogOfWarComponent->m_smoothedTextureData[k], smallIterationSize) == 0)
				{
					continue;
				}

				// Load values and do exponential decay smoothing.
				const __m256 target32s = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&sourceArray[k]))));
				const __m256 smoothed32s = _mm256_load_ps(&fogOfWarComponent->m_intermediarySmoothingData[k]);
				const __m256 resultFloats = _mm256_add_ps(target32s, _mm256_mul_ps(_mm256_sub_ps(smoothed32s, target32s), exponentialDecayCoefficient));

				// Store values back into intermediary smoothing array and final smoothed array.
				_mm256_store_ps(&fogOfWarComponent->m_intermediarySmoothingData[k], resultFloats);
				const __m256i packedInt8 = _mm256_permute4x64_epi64(_mm256_packs_epi32(_mm256_cvtps_epi32(resultFloats), _mm256_setzero_si256()), 0xD8);
				_mm_storel_epi64(reinterpret_cast<__m128i*>(&fogOfWarComponent->m_smoothedTextureData[k]), _mm_packus_epi16(_mm256_castsi256_si128(packedInt8), _mm256_extracti128_si256(packedInt8, 1)));
			}
		}
	}
}

void FogOfWarSystems::PopulateOffsetsForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, FogOfWarOffsets& outOffsets)
{
	ARGUS_TRACE(FogOfWarSystems::PopulateOffsetsForEntity);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const uint32 textureSize = static_cast<uint32>(fogOfWarComponent->m_textureSize);
	if (textureSize == 0u)
	{
		return;
	}

	const uint32 maxPixel = fogOfWarComponent->GetTotalPixels() - 1;
	const uint32 radius = GetPixelRadiusFromWorldSpaceRadius(fogOfWarComponent, components.m_targetingComponent->m_sightRange);
	const uint32 centerPixelRowNumber = components.m_fogOfWarLocationComponent->m_fogOfWarPixel / textureSize;

	outOffsets.m_leftOffset = radius;
	// The radius overlaps the left edge of the texture.
	if (((components.m_fogOfWarLocationComponent->m_fogOfWarPixel - outOffsets.m_leftOffset) / textureSize) != centerPixelRowNumber)
	{
		outOffsets.m_leftOffset = components.m_fogOfWarLocationComponent->m_fogOfWarPixel % textureSize;
	}

	outOffsets.m_rightOffset = radius;
	// The radius overlaps the right edge of the texture.
	if (((components.m_fogOfWarLocationComponent->m_fogOfWarPixel + outOffsets.m_rightOffset) / textureSize) != centerPixelRowNumber)
	{
		outOffsets.m_rightOffset = textureSize - (components.m_fogOfWarLocationComponent->m_fogOfWarPixel % textureSize);
	}

	outOffsets.m_topOffset = radius;
	// The radius overlaps the top edge of the texture.
	if (outOffsets.m_topOffset > centerPixelRowNumber)
	{
		outOffsets.m_topOffset = centerPixelRowNumber;
	}

	outOffsets.m_bottomOffset = radius;
	// The radius overlaps the bottom edge of the texture.
	if ((outOffsets.m_bottomOffset + centerPixelRowNumber) >= textureSize)
	{
		outOffsets.m_bottomOffset = ((maxPixel - (textureSize - (components.m_fogOfWarLocationComponent->m_fogOfWarPixel % textureSize))) / textureSize) - (centerPixelRowNumber);
	}
}

void FogOfWarSystems::PopulateOctantExpansionForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets, CircleOctantExpansion& outCircleOctantExpansion)
{
	ARGUS_TRACE(FogOfWarSystems::PopulateOctantExpansionForEntity);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	outCircleOctantExpansion.m_topY = offsets.m_circleX <= offsets.m_topOffset ? offsets.m_circleX : offsets.m_topOffset;
	outCircleOctantExpansion.m_topStartX = offsets.m_circleY <= offsets.m_leftOffset ? offsets.m_circleY : offsets.m_leftOffset;
	outCircleOctantExpansion.m_topEndX = offsets.m_circleY <= offsets.m_rightOffset ? offsets.m_circleY : offsets.m_rightOffset;

	outCircleOctantExpansion.m_bottomY = offsets.m_circleX <= offsets.m_bottomOffset ? offsets.m_circleX : offsets.m_bottomOffset;
	outCircleOctantExpansion.m_bottomStartX = offsets.m_circleY <= offsets.m_leftOffset ? offsets.m_circleY : offsets.m_leftOffset;
	outCircleOctantExpansion.m_bottomEndX = offsets.m_circleY <= offsets.m_rightOffset ? offsets.m_circleY : offsets.m_rightOffset;

	outCircleOctantExpansion.m_midUpY = offsets.m_circleY <= offsets.m_topOffset ? offsets.m_circleY : offsets.m_topOffset;
	outCircleOctantExpansion.m_midUpStartX = offsets.m_circleX <= offsets.m_leftOffset ? offsets.m_circleX : offsets.m_leftOffset;
	outCircleOctantExpansion.m_midUpEndX = offsets.m_circleX <= offsets.m_rightOffset ? offsets.m_circleX : offsets.m_rightOffset;

	outCircleOctantExpansion.m_midDownY = offsets.m_circleY <= offsets.m_bottomOffset ? offsets.m_circleY : offsets.m_bottomOffset;
	outCircleOctantExpansion.m_midDownStartX = offsets.m_circleX <= offsets.m_leftOffset ? offsets.m_circleX : offsets.m_leftOffset;
	outCircleOctantExpansion.m_midDownEndX = offsets.m_circleX <= offsets.m_rightOffset ? offsets.m_circleX : offsets.m_rightOffset;

	const uint32 textureSize = static_cast<uint32>(fogOfWarComponent->m_textureSize);
	outCircleOctantExpansion.m_centerColumnTopIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - (outCircleOctantExpansion.m_topY * textureSize);
	outCircleOctantExpansion.m_centerColumnMidUpIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - (outCircleOctantExpansion.m_midUpY * textureSize);
	outCircleOctantExpansion.m_centerColumnMidDownIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel + (outCircleOctantExpansion.m_midDownY * textureSize);
	outCircleOctantExpansion.m_centerColumnBottomIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel + (outCircleOctantExpansion.m_bottomY * textureSize);
}

void FogOfWarSystems::RevealPixelAlphaForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, FogOfWarOffsets& offsets, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::RevealPixelAlphaForEntity);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	TArray<ObstacleIndicies> obstacleIndicies;
	if (components.m_taskComponent->m_flightState == EFlightState::Grounded && activelyRevealed)
	{
		SpatialPartitioningComponent* spatialParitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
		ARGUS_RETURN_ON_NULL(spatialParitioningComponent, ArgusECSLog);

		spatialParitioningComponent->m_obstaclePointKDTree.FindObstacleIndiciesWithinRangeOfLocation
		(
			obstacleIndicies,
			ArgusMath::ToCartesianVector(components.m_transformComponent->m_location),
			components.m_targetingComponent->m_sightRange
		);
	}

	const uint32 radius = GetPixelRadiusFromWorldSpaceRadius(fogOfWarComponent, components.m_targetingComponent->m_sightRange);
	OctantTraces octantTraces;
	RasterizeCircleOfRadius(radius, offsets, [fogOfWarComponent, &components, &obstacleIndicies, &octantTraces, activelyRevealed](const FogOfWarOffsets& offsets)
	{
		// Set Alpha for pixel range for all symmetrical pixels.
		SetAlphaForCircleOctant(fogOfWarComponent, components, offsets, obstacleIndicies, octantTraces, activelyRevealed);
	});
}

void FogOfWarSystems::RasterizeCircleOfRadius(uint32 radius, FogOfWarOffsets& offsets, TFunction<void(FogOfWarOffsets& offsets)> perOctantPixelFunction)
{
	// Method of Horn for circle rasterization.
	int32 circleD = -static_cast<int32>(radius);
	offsets.m_circleX = radius;
	offsets.m_circleY = 0u;
	while (offsets.m_circleX >= offsets.m_circleY)
	{
		if (perOctantPixelFunction)
		{
			perOctantPixelFunction(offsets);
		}

		circleD = circleD + (2 * offsets.m_circleX * offsets.m_circleY) + 1u;
		offsets.m_circleY++;

		if (circleD > 0)
		{
			circleD = circleD - (2 * offsets.m_circleX * offsets.m_circleX) + 2;
			offsets.m_circleX--;
		}
	}
}

void FogOfWarSystems::SetAlphaForPixelRange(FogOfWarComponent* fogOfWarComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForPixelRange);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	if (activelyRevealed)
	{
		uint8* firstAddress = &fogOfWarComponent->m_textureData[fromPixelInclusive];
		uint32 rowLength = (toPixelInclusive - fromPixelInclusive) + 1;
		memset(firstAddress, 0, rowLength);
		return;
	}

	for (uint32 i = fromPixelInclusive; i <= toPixelInclusive; ++i)
	{
		if (fogOfWarComponent->m_textureData[i] != 0u)
		{
			continue;
		}

		fogOfWarComponent->m_textureData[i] = fogOfWarComponent->m_revealedOnceAlpha;
	}
}

void FogOfWarSystems::RevealPixelRangeWithObstacles(FogOfWarComponent* fogOfWarComponent, const SpatialPartitioningComponent* spatialPartitioningComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, const TArray<ObstacleIndicies>& obstacleIndicies, const FVector2D& cartesianEntityLocation, FVector2D& prevFrom, FVector2D& prevTo)
{
	ARGUS_TRACE(FogOfWarSystems::RevealPixelRangeWithObstacles);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const FVector2D cartesianFromLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, fromPixelInclusive));
	const FVector2D cartesianToLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, toPixelInclusive));

	for (int32 i = 0; i < obstacleIndicies.Num(); ++i)
	{
		const ObstaclePoint& currentObstaclePoint = spatialPartitioningComponent->m_obstacles[obstacleIndicies[i].m_obstacleIndex][obstacleIndicies[i].m_obstaclePointIndex];
		const ObstaclePoint& nextObstaclePoint = spatialPartitioningComponent->m_obstacles[obstacleIndicies[i].m_obstacleIndex].GetNext(obstacleIndicies[i].m_obstaclePointIndex);

		FVector2D fromIntersection = FVector2D::ZeroVector;
		FVector2D toIntersection = FVector2D::ZeroVector;
		ArgusMath::GetLineSegmentIntersectionCartesian(cartesianEntityLocation, cartesianFromLocation, currentObstaclePoint.m_point, nextObstaclePoint.m_point, fromIntersection);
		ArgusMath::GetLineSegmentIntersectionCartesian(cartesianEntityLocation, cartesianToLocation, currentObstaclePoint.m_point, nextObstaclePoint.m_point, toIntersection);
	}

	SetAlphaForPixelRange(fogOfWarComponent, fromPixelInclusive, toPixelInclusive, true);

	prevFrom = cartesianFromLocation;
	prevTo = cartesianToLocation;
}

void FogOfWarSystems::SetAlphaForCircleOctant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets, const TArray<ObstacleIndicies>& obstacleIndicies, OctantTraces& octantTraces, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForCircleOctant);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	CircleOctantExpansion octantExpansion;
	PopulateOctantExpansionForEntity(fogOfWarComponent, components, offsets, octantExpansion);

	if (obstacleIndicies.Num() > 0)
	{
		const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
		const FVector2D cartesianCenterLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, components.m_fogOfWarLocationComponent->m_fogOfWarPixel));
		RevealPixelRangeWithObstacles(fogOfWarComponent, spatialPartitioningComponent, octantExpansion.m_centerColumnTopIndex - octantExpansion.m_topStartX, octantExpansion.m_centerColumnTopIndex + octantExpansion.m_topEndX,
									  obstacleIndicies, cartesianCenterLocation, octantTraces.m_previousTopLeft, octantTraces.m_previousTopRight);

		RevealPixelRangeWithObstacles(fogOfWarComponent, spatialPartitioningComponent, octantExpansion.m_centerColumnMidUpIndex - octantExpansion.m_midUpStartX, octantExpansion.m_centerColumnMidUpIndex + octantExpansion.m_midUpEndX,
									  obstacleIndicies, cartesianCenterLocation, octantTraces.m_previousMidUpLeft, octantTraces.m_previousMidUpRight);

		RevealPixelRangeWithObstacles(fogOfWarComponent, spatialPartitioningComponent, octantExpansion.m_centerColumnMidDownIndex - octantExpansion.m_midDownStartX, octantExpansion.m_centerColumnMidDownIndex + octantExpansion.m_midDownEndX,
									  obstacleIndicies, cartesianCenterLocation, octantTraces.m_previousMidDownLeft, octantTraces.m_previousMidDownRight);

		RevealPixelRangeWithObstacles(fogOfWarComponent, spatialPartitioningComponent, octantExpansion.m_centerColumnBottomIndex - octantExpansion.m_bottomStartX, octantExpansion.m_centerColumnBottomIndex + octantExpansion.m_bottomEndX,
									  obstacleIndicies, cartesianCenterLocation, octantTraces.m_previousBottomLeft, octantTraces.m_previousBottomRight);
		return;
	}

	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnTopIndex - octantExpansion.m_topStartX, octantExpansion.m_centerColumnTopIndex + octantExpansion.m_topEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnMidUpIndex - octantExpansion.m_midUpStartX, octantExpansion.m_centerColumnMidUpIndex + octantExpansion.m_midUpEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnMidDownIndex - octantExpansion.m_midDownStartX, octantExpansion.m_centerColumnMidDownIndex + octantExpansion.m_midDownEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnBottomIndex - octantExpansion.m_bottomStartX, octantExpansion.m_centerColumnBottomIndex + octantExpansion.m_bottomEndX, activelyRevealed);
}

void FogOfWarSystems::UpdateTexture()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateTexture);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_fogOfWarTexture, ArgusECSLog);

	fogOfWarComponent->m_textureRegionsUpdateData = TextureRegionsUpdateData();

	FTextureResource* textureResource = fogOfWarComponent->m_fogOfWarTexture->GetResource();
	if (!textureResource)
	{
		return;
	}

	fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource = reinterpret_cast<FTexture2DResource*>(textureResource);
	fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI = fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource->GetTexture2DRHI();
	fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex = 0;
	fogOfWarComponent->m_textureRegionsUpdateData.m_numRegions = 1;
	fogOfWarComponent->m_textureRegionsUpdateData.m_regions = &fogOfWarComponent->m_textureRegion;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch = fogOfWarComponent->m_textureSize;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcBpp = 1;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcData = fogOfWarComponent->m_shouldUseSmoothing ? fogOfWarComponent->m_smoothedTextureData.GetData() : fogOfWarComponent->m_textureData.GetData();;

	if (!fogOfWarComponent->m_textureRegionsUpdateData.m_srcData)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
		[fogOfWarComponent](FRHICommandListImmediate& RHICmdList)
		{
			ARGUS_TRACE(FogOfWarSystems::ExecuteTextureUpdate);
			for (uint32 regionIndex = 0; regionIndex < fogOfWarComponent->m_textureRegionsUpdateData.m_numRegions; ++regionIndex)
			{
				int32 currentFirstMip = fogOfWarComponent->m_fogOfWarTexture->FirstResourceMemMip;
				if (fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI && fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex >= currentFirstMip)
				{
					RHIUpdateTexture2D(
						fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI,
						fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex - currentFirstMip,
						fogOfWarComponent->m_textureRegionsUpdateData.m_regions[regionIndex],
						fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch,
						fogOfWarComponent->m_textureRegionsUpdateData.m_srcData
						+ fogOfWarComponent->m_textureRegionsUpdateData.m_regions[regionIndex].SrcY * fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch
						+ fogOfWarComponent->m_textureRegionsUpdateData.m_regions[regionIndex].SrcX * fogOfWarComponent->m_textureRegionsUpdateData.m_srcBpp
					);
				}
			}
		});
}

void FogOfWarSystems::UpdateGaussianWeightsTexture()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateGaussianWeightsTexture);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	ENQUEUE_RENDER_COMMAND(UpdateGaussianWeightsTexture)(
		[fogOfWarComponent](FRHICommandListImmediate& RHICmdList)
		{
			ARGUS_TRACE(FogOfWarSystems::ExecuteGaussianWeightTextureUpdate);
			ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_gaussianWeightsTexture, ArgusECSLog);

			FUpdateTextureRegion2D region;
			region.SrcX = 0;
			region.SrcY = 0;
			region.DestX = 0;
			region.DestY = 0;
			region.Width = fogOfWarComponent->m_gaussianDimension;
			region.Height = fogOfWarComponent->m_gaussianDimension;

			uint32 srcPitch = fogOfWarComponent->m_gaussianDimension * sizeof(float);

			RHIUpdateTexture2D(
				reinterpret_cast<FTexture2DResource*>(fogOfWarComponent->m_gaussianWeightsTexture->GetResource())->GetTexture2DRHI(),
				0,
				region,
				fogOfWarComponent->m_gaussianDimension * sizeof(float),
				(const uint8*)fogOfWarComponent->m_gaussianFilter.GetData()
			);
		});
}

void FogOfWarSystems::UpdateDynamicMaterialInstance()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateDynamicMaterialInstance);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Set the dynamic material instance texture param to the fog of war texture.
	if (fogOfWarComponent->m_dynamicMaterialInstance && fogOfWarComponent->m_fogOfWarTexture && fogOfWarComponent->m_gaussianWeightsTexture)
	{
		fogOfWarComponent->m_dynamicMaterialInstance->SetTextureParameterValue("DynamicTexture", fogOfWarComponent->m_fogOfWarTexture);
		fogOfWarComponent->m_dynamicMaterialInstance->SetTextureParameterValue("GaussianWeights", fogOfWarComponent->m_gaussianWeightsTexture);
	}
}

uint32 FogOfWarSystems::GetPixelNumberFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector& worldSpaceLocation)
{
	ARGUS_RETURN_ON_NULL_UINT32(fogOfWarComponent, ArgusECSLog, 0u);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_UINT32(spatialPartitioningComponent, ArgusECSLog, 0u);

	const float textureSize = static_cast<float>(fogOfWarComponent->m_textureSize);
	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;

	float xValue = ArgusMath::SafeDivide(worldSpaceLocation.Y + spatialPartitioningComponent->m_validSpaceExtent, worldspaceWidth) * textureSize;
	float yValue = ArgusMath::SafeDivide((-worldSpaceLocation.X) + spatialPartitioningComponent->m_validSpaceExtent, worldspaceWidth) * textureSize;

	uint32 xValue32 = static_cast<uint32>(FMath::FloorToInt32(xValue));
	uint32 yValue32 = static_cast<uint32>(FMath::FloorToInt32(yValue));

	return (yValue32 * static_cast<uint32>(fogOfWarComponent->m_textureSize)) + xValue32;
}

FVector2D FogOfWarSystems::GetWorldSpaceLocationFromPixelNumber(FogOfWarComponent* fogOfWarComponent, uint32 pixelNumber)
{
	ARGUS_RETURN_ON_NULL_FVECTOR2D(fogOfWarComponent, ArgusECSLog, FVector2D::ZeroVector);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_FVECTOR2D(spatialPartitioningComponent, ArgusECSLog, FVector2D::ZeroVector);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float textureIncrement = ArgusMath::SafeDivide(worldspaceWidth, static_cast<float>(fogOfWarComponent->m_textureSize));

	const float leftOffset = static_cast<float>(pixelNumber % fogOfWarComponent->m_textureSize) * textureIncrement;
	const float topOffset = static_cast<float>(pixelNumber / fogOfWarComponent->m_textureSize) * textureIncrement;

	
	return FVector2D(spatialPartitioningComponent->m_validSpaceExtent - topOffset, leftOffset - spatialPartitioningComponent->m_validSpaceExtent);
}

uint32 FogOfWarSystems::GetPixelRadiusFromWorldSpaceRadius(FogOfWarComponent* fogOfWarComponent, float radius)
{
	ARGUS_RETURN_ON_NULL_UINT32(fogOfWarComponent, ArgusECSLog, 0u);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_UINT32(spatialPartitioningComponent, ArgusECSLog, 0u);

	const float portion = ArgusMath::SafeDivide(radius, (2.0f * spatialPartitioningComponent->m_validSpaceExtent));
	return FMath::FloorToInt32(static_cast<float>(fogOfWarComponent->m_textureSize) * portion);
}

void FogOfWarSystems::UpdateDoesEntityNeedToUpdateActivelyRevealed(const FogOfWarSystemsArgs& components, const InputInterfaceComponent* inputInterfaceComponent)
{
	ARGUS_TRACE(FogOfWarSystems::DoesEntityNeedToUpdateActivelyRevealed);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	if (!components.m_entity.IsAlive() || !components.m_entity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam) || components.m_entity.IsPassenger())
	{
		components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = false;
		return;
	}

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const TFunction<bool(const ArgusEntityKDTreeNode*)> queryFilter = [&components, inputInterfaceComponent](const ArgusEntityKDTreeNode* entityNode)
	{
		ARGUS_RETURN_ON_NULL_BOOL(entityNode, ArgusECSLog);
		ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusECSLog);
		if (entityNode->m_entityId == components.m_entity.GetId())
		{
			return false;
		}

		ArgusEntity otherEntity = ArgusEntity::RetrieveEntity(entityNode->m_entityId);
		if (!otherEntity || !otherEntity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam) || otherEntity.IsPassenger())
		{
			return false;
		}

		// If the other entity is dead, but was registered dead to FogOfWar this frame, that means that the revealed once state might be clipping actively revealed areas.
		if (!otherEntity.IsAlive())
		{
			if (const FogOfWarLocationComponent* otherFogOfWarLocationComponent = otherEntity.GetComponent<FogOfWarLocationComponent>())
			{
				if (otherFogOfWarLocationComponent->m_clearedThisFrame)
				{
					return true;
				}
			}

			return false;
		}

		if (const VelocityComponent* velocityComponent = otherEntity.GetComponent<VelocityComponent>())
		{
			if (!velocityComponent->m_currentVelocity.IsNearlyZero() || !velocityComponent->m_proposedAvoidanceVelocity.IsNearlyZero())
			{
				return true;
			}
		}

		return false;
	};

	uint16 nearestMovingGroundedTeammateId = spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdClosestToArgusEntity(components.m_entity, queryFilter);
	uint16 nearestMovingFlyingTeammateId = spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindOtherArgusEntityIdClosestToArgusEntity(components.m_entity, queryFilter);
	if (nearestMovingGroundedTeammateId == ArgusECSConstants::k_maxEntities && nearestMovingFlyingTeammateId == ArgusECSConstants::k_maxEntities)
	{
		components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = false;
		return;
	}

	FogOfWarSystemsArgs groundedEntityComponents;
	FogOfWarSystemsArgs flyingEntityComponents;

	const bool validGroundedEntity = groundedEntityComponents.PopulateArguments(ArgusEntity::RetrieveEntity(nearestMovingGroundedTeammateId));
	const bool validFlyingEntity = flyingEntityComponents.PopulateArguments(ArgusEntity::RetrieveEntity(nearestMovingFlyingTeammateId));
	if (!validGroundedEntity && !validFlyingEntity)
	{
		components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = false;
		return;
	}

	bool withinGroundedRange = false;
	bool withinFlyingRange = false;
	if (validGroundedEntity)
	{
		const float groundedDistSquared = FVector::DistSquared2D(components.m_transformComponent->m_location, groundedEntityComponents.m_transformComponent->m_location);
		const float groundedRadiusSquared = FMath::Square(components.m_targetingComponent->m_sightRange + groundedEntityComponents.m_targetingComponent->m_sightRange);
		withinGroundedRange = groundedDistSquared < groundedRadiusSquared;
	}

	if (validFlyingEntity)
	{
		const float flyingDistSquared = FVector::DistSquared2D(components.m_transformComponent->m_location, flyingEntityComponents.m_transformComponent->m_location);
		const float flyingRadiusSquared = FMath::Square(components.m_targetingComponent->m_sightRange + flyingEntityComponents.m_targetingComponent->m_sightRange);
		withinFlyingRange = flyingDistSquared < flyingRadiusSquared;
	}

	components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = (withinGroundedRange || withinFlyingRange);
}

bool FogOfWarSystems::IsPixelInFogOfWarBounds(int32 relativeX, int32 relativeY, FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components)
{
	ARGUS_TRACE(FogOfWarSystems::IsPixelInFogOfWarBounds);
	ARGUS_RETURN_ON_NULL_BOOL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	const int32 textureSize = static_cast<int32>(fogOfWarComponent->m_textureSize);
	if (textureSize == 0u)
	{
		return false;
	}

	const int32 maxPixel = fogOfWarComponent->GetTotalPixels() - 1;
	const int32 centerPixelRowNumber = components.m_fogOfWarLocationComponent->m_fogOfWarPixel / textureSize;

	// The radius overlaps the right edge of the texture.
	if (((components.m_fogOfWarLocationComponent->m_fogOfWarPixel + relativeX) / textureSize) != centerPixelRowNumber)
	{
		return false;
	}

	// The radius overlaps the top edge of the texture.
	if (relativeY > centerPixelRowNumber)
	{
		return false;
	}

	// The radius overlaps the bottom edge of the texture.
	if (relativeY < 0 && (centerPixelRowNumber - relativeY) >= textureSize)
	{
		return false;
	}

	return true;
}

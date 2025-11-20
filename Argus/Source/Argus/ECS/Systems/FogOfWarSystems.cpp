// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FogOfWarSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Async/ParallelFor.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "SystemArgumentDefinitions/FogOfWarSystemsArgs.h"

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

	// Iterate over entire texture and set any pixels that are actively revealed to revealed once.
	ClearActivelyRevealedPixels(fogOfWarComponent);

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

void FogOfWarSystems::ClearActivelyRevealedPixels(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_TRACE(FogOfWarSystems::ClearActivelyRevealedPixels);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	const int32 chunkSize = ArgusMath::SafeDivide<int32>(static_cast<int32>(fogOfWarComponent->GetTotalPixels()), fogOfWarComponent->m_numberSmoothingChunks, static_cast<int32>(fogOfWarComponent->GetTotalPixels()));
	int32 currentStartIndex = 0;

	fogOfWarComponent->m_asyncTasks.Reset();

	for (int32 i = 0; i < fogOfWarComponent->m_numberSmoothingChunks; ++i)
	{
		fogOfWarComponent->m_asyncTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(FogOfWarSystems::ClearActivelyRevealedPixelsForRange), [fogOfWarComponent, currentStartIndex, chunkSize]()
		{
			ClearActivelyRevealedPixelsForRange(fogOfWarComponent, currentStartIndex, currentStartIndex + chunkSize);
		}));
		currentStartIndex += chunkSize;
	}

	UE::Tasks::Wait(fogOfWarComponent->m_asyncTasks);
}

void FogOfWarSystems::ClearActivelyRevealedPixelsForRange(FogOfWarComponent* fogOfWarComponent, int32 fromInclusive, int32 toExclusive)
{
	ARGUS_TRACE(FogOfWarSystems::ClearActivelyRevealedPixelsForRange);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	static constexpr int32 topIterationSize = 256;
	static constexpr int32 midIterationSize = 32;
	static constexpr int32 smallIterationSize = 8;

	for (int32 i = fromInclusive; i < toExclusive; i += topIterationSize)
	{
		if (!memchr(&fogOfWarComponent->m_textureData[i], 0, topIterationSize))
		{
			continue;
		}

		for (int32 j = i; j < i + topIterationSize; j += midIterationSize)
		{
			if (!memchr(&fogOfWarComponent->m_textureData[j], 0, midIterationSize))
			{
				continue;
			}

			for (int32 k = j; k < j + midIterationSize; k += smallIterationSize)
			{
				if (!memchr(&fogOfWarComponent->m_textureData[k], 0, smallIterationSize))
				{
					continue;
				}

				for (int32 l = k; l < k + smallIterationSize; ++l)
				{
					if (fogOfWarComponent->m_textureData[l] == 0u)
					{
						fogOfWarComponent->m_textureData[l] = fogOfWarComponent->m_revealedOnceAlpha;
					}
				}
			}
		}
	}
}

void FogOfWarSystems::SetRevealedStatePerEntity(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_TRACE(FogOfWarSystems::SetRevealedStatePerEntity);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	fogOfWarComponent->m_asyncTasks.Reset();

	// Calculate new actively revealed pixels.
	ARGUS_TRACE(FogOfWarSystems::ActivelyReveal)
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(entity))
		{
			continue;
		}

		if (!entity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam) || !components.m_entity.IsAlive())
		{
			continue;
		}

		components.m_fogOfWarLocationComponent->m_fogOfWarPixel = GetPixelNumberFromWorldSpaceLocation(fogOfWarComponent, components.m_transformComponent->m_location);
		RevealPixelAlphaForEntity(fogOfWarComponent, i);
	}

	UE::Tasks::Wait(fogOfWarComponent->m_asyncTasks);
}

void FogOfWarSystems::ApplyExponentialDecaySmoothing(FogOfWarComponent* fogOfWarComponent, float deltaTime)
{
	ARGUS_TRACE(FogOfWarSystems::ApplyExponentialDecaySmoothing);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (FMath::IsNearlyZero(deltaTime))
	{
		return;
	}

	const __m256 exponentialDecayCoefficient = _mm256_set1_ps(FMath::Exp(-fogOfWarComponent->m_smoothingDecayConstant * deltaTime));
	const int32 chunkSize = ArgusMath::SafeDivide<int32>(static_cast<int32>(fogOfWarComponent->GetTotalPixels()), fogOfWarComponent->m_numberSmoothingChunks, static_cast<int32>(fogOfWarComponent->GetTotalPixels()));
	int32 currentStartIndex = 0;

	fogOfWarComponent->m_asyncTasks.Reset();

	for (int32 i = 0; i < fogOfWarComponent->m_numberSmoothingChunks; ++i)
	{
		fogOfWarComponent->m_asyncTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(FogOfWarSystems::RevealPixelAlphaForEntity), [fogOfWarComponent, deltaTime, &exponentialDecayCoefficient, currentStartIndex, chunkSize]()
		{
			ApplyExponentialDecaySmoothingForRange(fogOfWarComponent, deltaTime, exponentialDecayCoefficient, currentStartIndex, currentStartIndex + chunkSize);
		}));
		currentStartIndex += chunkSize;
	}

	UE::Tasks::Wait(fogOfWarComponent->m_asyncTasks);
}

void FogOfWarSystems::ApplyExponentialDecaySmoothingForRange(FogOfWarComponent* fogOfWarComponent, float deltaTime, const __m256& exponentialDecayCoefficient, int32 fromInclusive, int32 toExclusive)
{
	ARGUS_TRACE(FogOfWarSystems::ApplyExponentialDecaySmoothingForRange);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	static constexpr int32 topIterationSize = 128;
	static constexpr int32 midIterationSize = 32;
	static constexpr int32 smallIterationSize = 8;
	const uint8* sourceArray = fogOfWarComponent->m_textureData.GetData();

	// value = targetValue + ((value - targetValue) * FMath::Exp(-decayConstant * deltaTime));
	for (int32 i = fromInclusive; i < toExclusive; i += topIterationSize)
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

void FogOfWarSystems::RevealPixelAlphaForEntity(FogOfWarComponent* fogOfWarComponent, uint16 entityId)
{
	ARGUS_TRACE(FogOfWarSystems::RevealPixelAlphaForEntity);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	FogOfWarSystemsArgs components;
	if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(entityId)))
	{
		return;
	}

	uint32 radius = GetPixelRadiusFromWorldSpaceRadius(fogOfWarComponent, components.m_targetingComponent->m_sightRange);
	const FVector2D initialLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, components.m_fogOfWarLocationComponent->m_fogOfWarPixel));

#pragma region Top Task
	fogOfWarComponent->m_asyncTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(FogOfWarSystems::SetAlphaForCircleQuadrant), [fogOfWarComponent, entityId, radius, initialLocation]()
	{
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(entityId)))
		{
			return;
		}

		TArray<ObstacleIndicies> obstacleIndicies;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded)
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

		uint32 modifiedRadius = radius;
		if (obstacleIndicies.Num() > 0)
		{
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces topTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, obstacleIndicies.Num() > 0, [fogOfWarComponent, &components, &obstacleIndicies, &topTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleX <= offsets.m_topOffset ? offsets.m_circleX : offsets.m_topOffset;
			quadrant.m_xStartValue = offsets.m_circleY <= offsets.m_leftOffset ? offsets.m_circleY : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleY <= offsets.m_rightOffset ? offsets.m_circleY : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, obstacleIndicies, topTraces);
		});
	}));
#pragma endregion

#pragma region Mid Up Task
	fogOfWarComponent->m_asyncTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(FogOfWarSystems::SetAlphaForCircleQuadrant), [fogOfWarComponent, entityId, radius, initialLocation]()
	{
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(entityId)))
		{
			return;
		}

		TArray<ObstacleIndicies> obstacleIndicies;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded)
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

		uint32 modifiedRadius = radius;
		if (obstacleIndicies.Num() > 0)
		{
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces midUpTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, obstacleIndicies.Num() > 0, [fogOfWarComponent, &components, &obstacleIndicies, &midUpTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleY <= offsets.m_topOffset ? offsets.m_circleY : offsets.m_topOffset;
			quadrant.m_xStartValue = offsets.m_circleX <= offsets.m_leftOffset ? offsets.m_circleX : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleX <= offsets.m_rightOffset ? offsets.m_circleX : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, obstacleIndicies, midUpTraces);
		});
	}));
#pragma endregion

#pragma region Mid Down Task
	fogOfWarComponent->m_asyncTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(FogOfWarSystems::SetAlphaForCircleQuadrant), [fogOfWarComponent, entityId, radius, initialLocation]()
	{
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(entityId)))
		{
			return;
		}

		TArray<ObstacleIndicies> obstacleIndicies;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded)
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

		uint32 modifiedRadius = radius;
		if (obstacleIndicies.Num() > 0)
		{
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces midDownTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, obstacleIndicies.Num() > 0, [fogOfWarComponent, &components, &obstacleIndicies, &midDownTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleY <= offsets.m_bottomOffset ? offsets.m_circleY : offsets.m_bottomOffset;
			quadrant.m_xStartValue = offsets.m_circleX <= offsets.m_leftOffset ? offsets.m_circleX : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleX <= offsets.m_rightOffset ? offsets.m_circleX : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel + (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, obstacleIndicies, midDownTraces);
		});
	}));
#pragma endregion

#pragma region Bottom Task
	fogOfWarComponent->m_asyncTasks.Add(UE::Tasks::Launch(ARGUS_NAMEOF(FogOfWarSystems::SetAlphaForCircleQuadrant), [fogOfWarComponent, entityId, radius, initialLocation]()
	{
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(entityId)))
		{
			return;
		}

		TArray<ObstacleIndicies> obstacleIndicies;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded)
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

		uint32 modifiedRadius = radius;
		if (obstacleIndicies.Num() > 0)
		{
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces bottomTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, obstacleIndicies.Num() > 0, [fogOfWarComponent, &components, &obstacleIndicies, &bottomTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleX <= offsets.m_bottomOffset ? offsets.m_circleX : offsets.m_bottomOffset;
			quadrant.m_xStartValue = offsets.m_circleY <= offsets.m_leftOffset ? offsets.m_circleY : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleY <= offsets.m_rightOffset ? offsets.m_circleY : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel + (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, obstacleIndicies, bottomTraces);
		});
	}));
#pragma endregion
}

void FogOfWarSystems::RasterizeCircleOfRadius(FogOfWarComponent* fogOfWarComponent, uint32 radius, FogOfWarOffsets& offsets, bool accountForTriangleRasterization, TFunction<void(FogOfWarOffsets& offsets)> perOctantPixelFunction)
{
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Method of Horn for circle rasterization.
	int32 circleD = -static_cast<int32>(radius);
	offsets.m_circleX = radius;
	offsets.m_circleY = 0u;

	uint32 margin = accountForTriangleRasterization ? fogOfWarComponent->m_triangleRasterizeModulo : 0u;
	while ((offsets.m_circleX + margin + margin) >= offsets.m_circleY)
	{
		if (perOctantPixelFunction && (!accountForTriangleRasterization || ((offsets.m_circleY % fogOfWarComponent->m_triangleRasterizeModulo) == 0u)) || (offsets.m_circleX + margin + margin) == offsets.m_circleY)
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

void FogOfWarSystems::RasterizeTriangleForReveal(FogOfWarComponent* fogOfWarComponent, const FVector2D& point0, const FVector2D& point1, const FVector2D& point2)
{
	ARGUS_TRACE(FogOfWarSystems::RasterizeTriangleForReveal);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	TArray<TPair<int32, int32>, TInlineAllocator<3>> points;
	points.SetNumUninitialized(3);
	GetPixelCoordsFromWorldSpaceLocation(fogOfWarComponent, ArgusMath::ToUnrealVector2(point0), points[0]);
	GetPixelCoordsFromWorldSpaceLocation(fogOfWarComponent, ArgusMath::ToUnrealVector2(point1), points[1]);
	GetPixelCoordsFromWorldSpaceLocation(fogOfWarComponent, ArgusMath::ToUnrealVector2(point2), points[2]);

	points.Sort([](const TPair<int32, int32>& pointA, const TPair<int32, int32>& pointB)
	{
		if (pointA.Value == pointB.Value)
		{
			return pointA.Key < pointB.Key;
		}

		return pointA.Value > pointB.Value;
	});

	if (points[1].Value == points[2].Value)
	{
		FillFlatBottomTriangle(fogOfWarComponent, points[0], points[1], points[2]);
		return;
	}

	if (points[0].Value == points[1].Value)
	{
		FillFlatTopTriangle(fogOfWarComponent, points[0], points[1], points[2]);
		return;
	}

	if (points[2].Value == points[0].Value)
	{
		return;
	}

	// Add a fourth vertex to split the triangle into two triangles one with a flat bottom and the other with a flat top.
	TPair<int32, int32> point3;
	point3.Value = points[1].Value;
	const float dividend = static_cast<float>(points[1].Value - points[0].Value) / static_cast<float>(points[2].Value - points[0].Value);
	point3.Key = FMath::RoundToInt32((dividend * static_cast<float>(points[2].Key - points[0].Key)) + static_cast<float>(points[0].Key));

	if (point3.Key > points[1].Key)
	{
		FillFlatBottomTriangle(fogOfWarComponent, points[0], points[1], point3);
		FillFlatTopTriangle(fogOfWarComponent, points[1], point3, points[2]);
	}
	else
	{
		FillFlatBottomTriangle(fogOfWarComponent, points[0], point3, points[1]);
		FillFlatTopTriangle(fogOfWarComponent, point3, points[1], points[2]);
	}
}

void FogOfWarSystems::FillFlatBottomTriangle(FogOfWarComponent* fogOfWarComponent, const TPair<int32, int32>& point0, const TPair<int32, int32>& point1, const TPair<int32, int32>& point2)
{
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	if (point0.Value == point1.Value || point0.Value == point2.Value)
	{
		return;
	}

	const float inverseSlopeLeft = static_cast<float>(point1.Key - point0.Key) / static_cast<float>(point1.Value - point0.Value);
	const float inverseSlopeRight = static_cast<float>(point2.Key - point0.Key) / static_cast<float>(point2.Value - point0.Value);

	float leftEdgeX = point0.Key;
	float rightEdgeX = point0.Key;

	for (int32 height = point0.Value; height >= point1.Value; --height)
	{
		const uint32 heightIndex = static_cast<uint32>(height) * static_cast<uint32>(fogOfWarComponent->m_textureSize);
		const uint32 leftIndex = FMath::FloorToInt32(leftEdgeX);
		const uint32 rightIndex = FMath::CeilToInt32(rightEdgeX);
		SetAlphaForPixelRange(fogOfWarComponent, heightIndex + leftIndex, heightIndex + rightIndex);

		leftEdgeX -= inverseSlopeLeft;
		rightEdgeX -= inverseSlopeRight;
	}
}

void FogOfWarSystems::FillFlatTopTriangle(FogOfWarComponent* fogOfWarComponent, const TPair<int32, int32>& point0, const TPair<int32, int32>& point1, const TPair<int32, int32>& point2)
{
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	if (point2.Value == point0.Value || point2.Value == point1.Value)
	{
		return;
	}

	const float inverseSlopeLeft = static_cast<float>(point2.Key - point0.Key) / static_cast<float>(point2.Value - point0.Value);
	const float inverseSlopeRight = static_cast<float>(point2.Key - point1.Key) / static_cast<float>(point2.Value - point1.Value);

	float leftEdgeX = point2.Key;
	float rightEdgeX = point2.Key;

	for (int32 height = point2.Value; height <= point0.Value; ++height)
	{
		const uint32 heightIndex = static_cast<uint32>(height) * static_cast<uint32>(fogOfWarComponent->m_textureSize);
		const uint32 leftIndex = FMath::FloorToInt32(leftEdgeX);
		const uint32 rightIndex = FMath::CeilToInt32(rightEdgeX);
		SetAlphaForPixelRange(fogOfWarComponent, heightIndex + leftIndex, heightIndex + rightIndex);

		leftEdgeX += inverseSlopeLeft;
		rightEdgeX += inverseSlopeRight;
	}
}

void FogOfWarSystems::SetAlphaForPixelRange(FogOfWarComponent* fogOfWarComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForPixelRange);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	uint8* firstAddress = &fogOfWarComponent->m_textureData[fromPixelInclusive];
	uint32 rowLength = (toPixelInclusive - fromPixelInclusive) + 1;
	memset(firstAddress, 0, rowLength);
	return;
}

void FogOfWarSystems::RevealPixelRangeWithObstacles(FogOfWarComponent* fogOfWarComponent, const SpatialPartitioningComponent* spatialPartitioningComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, const TArray<ObstacleIndicies>& obstacleIndicies, const FVector2D& cartesianEntityLocation, FVector2D& prevFrom, FVector2D& prevTo)
{
	ARGUS_TRACE(FogOfWarSystems::RevealPixelRangeWithObstacles);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const FVector2D cartesianFromLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, fromPixelInclusive));
	const FVector2D cartesianToLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, toPixelInclusive));

	FVector2D currentFromIntersection = cartesianFromLocation;
	FVector2D currentToIntersection = cartesianToLocation;

	for (int32 i = 0; i < obstacleIndicies.Num(); ++i)
	{
		const ObstaclePoint& currentObstaclePoint = spatialPartitioningComponent->m_obstacles[obstacleIndicies[i].m_obstacleIndex][obstacleIndicies[i].m_obstaclePointIndex];
		const ObstaclePoint& nextObstaclePoint = spatialPartitioningComponent->m_obstacles[obstacleIndicies[i].m_obstacleIndex].GetNext(obstacleIndicies[i].m_obstaclePointIndex);

		FVector2D currentPoint = currentObstaclePoint.m_point;
		const FVector2D currentLeft = currentObstaclePoint.GetLeftVector();
		FVector2D nextPoint = nextObstaclePoint.m_point;
		const FVector2D nextLeft = nextObstaclePoint.GetLeftVector();

		currentPoint += (currentLeft * fogOfWarComponent->m_visionObstacleAdjustDistance);
		nextPoint += (nextLeft * fogOfWarComponent->m_visionObstacleAdjustDistance);

		FVector2D fromIntersection = cartesianFromLocation;
		FVector2D toIntersection = cartesianToLocation;

		if (ArgusMath::IsLeftOfCartesian(currentPoint, nextPoint, cartesianEntityLocation))
		{
			continue;
		}

		if (ArgusMath::GetLineSegmentIntersectionCartesian(cartesianEntityLocation, cartesianFromLocation, currentPoint, nextPoint, fromIntersection))
		{
			if (FVector2D::DistSquared(cartesianEntityLocation, fromIntersection) < FVector2D::DistSquared(cartesianEntityLocation, currentFromIntersection))
			{
				currentFromIntersection = fromIntersection;
			}
		}

		if (ArgusMath::GetLineSegmentIntersectionCartesian(cartesianEntityLocation, cartesianToLocation, currentPoint, nextPoint, toIntersection))
		{
			if (FVector2D::DistSquared(cartesianEntityLocation, toIntersection) < FVector2D::DistSquared(cartesianEntityLocation, currentToIntersection))
			{
				currentToIntersection = toIntersection;
			}
		}
	}

	if (prevFrom != cartesianEntityLocation)
	{
		RasterizeTriangleForReveal(fogOfWarComponent, cartesianEntityLocation, prevFrom, currentFromIntersection);
	}
	if (prevTo != cartesianEntityLocation)
	{
		RasterizeTriangleForReveal(fogOfWarComponent, cartesianEntityLocation, prevTo, currentToIntersection);
	}

	prevFrom = currentFromIntersection;
	prevTo = currentToIntersection;
}

void FogOfWarSystems::SetAlphaForCircleQuadrant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const CircleQuadrant& quadrant, const TArray<ObstacleIndicies>& obstacleIndicies, QuadrantObstacleTraces& quadrantTraces)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForCircleQuadrant);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (obstacleIndicies.Num() > 0)
	{
		const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
		const FVector2D cartesianCenterLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, components.m_fogOfWarLocationComponent->m_fogOfWarPixel));
		RevealPixelRangeWithObstacles(fogOfWarComponent, spatialPartitioningComponent, quadrant.m_centerColumnIndex - quadrant.m_xStartValue, quadrant.m_centerColumnIndex + quadrant.m_xEndValue,
			obstacleIndicies, cartesianCenterLocation, quadrantTraces.m_previousLeft, quadrantTraces.m_previousRight);

		return;
	}

	SetAlphaForPixelRange(fogOfWarComponent, quadrant.m_centerColumnIndex - quadrant.m_xStartValue, quadrant.m_centerColumnIndex + quadrant.m_xEndValue);
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

bool FogOfWarSystems::GetPixelCoordsFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const FVector2D& worldSpaceLocation, TPair<int32, int32>& ouputPair)
{
	ARGUS_RETURN_ON_NULL_BOOL(fogOfWarComponent, ArgusECSLog);

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(spatialPartitioningComponent, ArgusECSLog);

	const float textureSize = static_cast<float>(fogOfWarComponent->m_textureSize);
	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;

	float xValue = ArgusMath::SafeDivide(worldSpaceLocation.Y + spatialPartitioningComponent->m_validSpaceExtent, worldspaceWidth) * textureSize;
	float yValue = ArgusMath::SafeDivide((-worldSpaceLocation.X) + spatialPartitioningComponent->m_validSpaceExtent, worldspaceWidth) * textureSize;

	ouputPair.Key = FMath::FloorToInt32(xValue);
	ouputPair.Value = FMath::FloorToInt32(yValue);

	return true;
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

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
	ARGUS_TRACE(FogOfWarSystems::RunThreadSystems);

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

	uint8* textureData = fogOfWarComponent->m_textureData.GetData();
	const uint8 revealedOnceAlpha = fogOfWarComponent->m_revealedOnceAlpha;

	const __m256i zeroedBytes = _mm256_setzero_si256();
	const __m256i replacementAlphaBytes = _mm256_set1_epi8(static_cast<char>(revealedOnceAlpha));

	// Ensure we don't exceed bounds
	const int32 alignedEnd = fromInclusive + ((toExclusive - fromInclusive) / topIterationSize) * topIterationSize;
	for (int32 i = fromInclusive; i < alignedEnd; i += topIterationSize)
	{
		if (!memchr(&textureData[i], 0, topIterationSize))
		{
			continue;
		}

		for (int32 j = i; j < i + topIterationSize; j += midIterationSize)
		{
			if (!memchr(&textureData[j], 0, midIterationSize))
			{
				continue;
			}

			// SIMD: Load 32 bytes, find zeros, replace with revealedOnceAlpha
			__m256i data = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&textureData[j]));

			// Create mask where zeros are 0xFF
			__m256i zeroMask = _mm256_cmpeq_epi8(data, zeroedBytes);

			// Blend: keep original where not zero, use replacement where zero
			__m256i result = _mm256_blendv_epi8(data, replacementAlphaBytes, zeroMask);

			// Store result
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(&textureData[j]), result);
		}
	}

	for (int32 i = alignedEnd; i < toExclusive; ++i)
	{
		if (textureData[i] == 0u)
		{
			textureData[i] = revealedOnceAlpha;
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
	ArgusEntity::IterateSystemsArgs<FogOfWarSystemsArgs>([fogOfWarComponent, inputInterfaceComponent](FogOfWarSystemsArgs& components)
	{
		if (!components.m_entity.IsOnTeam(inputInterfaceComponent->m_activePlayerTeam) || !components.m_entity.IsAlive() || components.m_entity.IsUnderConstruction())
		{
			return;
		}

		components.m_fogOfWarLocationComponent->m_fogOfWarPixel = GetPixelNumberFromWorldSpaceLocation(fogOfWarComponent, components.m_transformComponent->m_location);
		RevealPixelAlphaForEntity(fogOfWarComponent, components.m_entity.GetId());
	});

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
	static constexpr int32 smallIterationSize = 8;
	const uint8* sourceArray = fogOfWarComponent->m_textureData.GetData();
	uint8* smoothedArray = fogOfWarComponent->m_smoothedTextureData.GetData();
	float* intermediaryArray = fogOfWarComponent->m_intermediarySmoothingData.GetData();

	// value = targetValue + ((value - targetValue) * FMath::Exp(-decayConstant * deltaTime));
	const int32 alignedEnd = fromInclusive + ((toExclusive - fromInclusive) / topIterationSize) * topIterationSize;
	for (int32 i = fromInclusive; i < alignedEnd; i += topIterationSize)
	{
		if (memcmp(&sourceArray[i], &smoothedArray[i], topIterationSize) == 0)
		{
			continue;
		}

		for (int32 j = i; j < i + topIterationSize; j += smallIterationSize)
		{
			if (*reinterpret_cast<const uint64*>(&sourceArray[j]) == *reinterpret_cast<const uint64*>(&smoothedArray[j]))
			{
				continue;
			}

			// Load values and do exponential decay smoothing.
			const __m256 target32s = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&sourceArray[j]))));
			const __m256 smoothed32s = _mm256_load_ps(&intermediaryArray[j]);
			const __m256 diff = _mm256_sub_ps(smoothed32s, target32s);
			const __m256 resultFloats = _mm256_fmadd_ps(diff, exponentialDecayCoefficient, target32s);

			// Store values back into intermediary smoothing array and final smoothed array.
			_mm256_store_ps(&intermediaryArray[j], resultFloats);

			// Convert floats to uint8 and store
			// Step 1: float -> int32
			const __m256i resultInt32 = _mm256_cvtps_epi32(resultFloats);

			// Step 2: int32 -> int16 (pack with saturation)
			const __m256i packed16 = _mm256_packs_epi32(resultInt32, _mm256_setzero_si256());

			// Step 3: Reorder lanes and extract lower 64 bits as uint8
			const __m256i reordered = _mm256_permute4x64_epi64(packed16, 0xD8);
			const __m128i lower128 = _mm256_castsi256_si128(reordered);
			const __m128i upper128 = _mm256_extracti128_si256(reordered, 1);
			const __m128i packed8 = _mm_packus_epi16(lower128, upper128);

			_mm_storel_epi64(reinterpret_cast<__m128i*>(&smoothedArray[j]), packed8);
		}
	}

	for (int32 i = alignedEnd; i < toExclusive; ++i)
	{
		if (sourceArray[i] == smoothedArray[i])
		{
			continue;
		}

		const float target = static_cast<float>(sourceArray[i]);
		const float smoothed = intermediaryArray[i];
		const float result = target + ((smoothed - target) * FMath::Exp(-fogOfWarComponent->m_smoothingDecayConstant * deltaTime));

		intermediaryArray[i] = result;
		smoothedArray[i] = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt32(result), 0, 255));
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
		outOffsets.m_rightOffset = textureSize - ((components.m_fogOfWarLocationComponent->m_fogOfWarPixel % textureSize) + 1u);
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

		bool nearObstacles = false;
		uint32 modifiedRadius = radius;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded && components.m_nearbyObstaclesComponent && components.m_nearbyObstaclesComponent->m_obstacleIndicies.GetInRangeObstacleIndicies().Num())
		{
			nearObstacles = true;
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces topTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, nearObstacles, [fogOfWarComponent, nearObstacles, &components, &topTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleX <= offsets.m_topOffset ? offsets.m_circleX : offsets.m_topOffset;
			quadrant.m_xStartValue = offsets.m_circleY <= offsets.m_leftOffset ? offsets.m_circleY : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleY <= offsets.m_rightOffset ? offsets.m_circleY : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, nearObstacles, topTraces);
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

		bool nearObstacles = false;
		uint32 modifiedRadius = radius;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded && components.m_nearbyObstaclesComponent && components.m_nearbyObstaclesComponent->m_obstacleIndicies.GetInRangeObstacleIndicies().Num())
		{
			nearObstacles = true;
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces midUpTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, nearObstacles, [fogOfWarComponent, nearObstacles, &components, &midUpTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleY <= offsets.m_topOffset ? offsets.m_circleY : offsets.m_topOffset;
			quadrant.m_xStartValue = offsets.m_circleX <= offsets.m_leftOffset ? offsets.m_circleX : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleX <= offsets.m_rightOffset ? offsets.m_circleX : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, nearObstacles, midUpTraces);
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

		bool nearObstacles = false;
		uint32 modifiedRadius = radius;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded && components.m_nearbyObstaclesComponent && components.m_nearbyObstaclesComponent->m_obstacleIndicies.GetInRangeObstacleIndicies().Num())
		{
			nearObstacles = true;
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces midDownTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, nearObstacles, [fogOfWarComponent, nearObstacles, &components, &midDownTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleY <= offsets.m_bottomOffset ? offsets.m_circleY : offsets.m_bottomOffset;
			quadrant.m_xStartValue = offsets.m_circleX <= offsets.m_leftOffset ? offsets.m_circleX : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleX <= offsets.m_rightOffset ? offsets.m_circleX : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel + (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, nearObstacles, midDownTraces);
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

		bool nearObstacles = false;
		uint32 modifiedRadius = radius;
		if (components.m_taskComponent->m_flightState == EFlightState::Grounded && components.m_nearbyObstaclesComponent && components.m_nearbyObstaclesComponent->m_obstacleIndicies.GetInRangeObstacleIndicies().Num())
		{
			nearObstacles = true;
			modifiedRadius--;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		QuadrantObstacleTraces bottomTraces = QuadrantObstacleTraces(initialLocation);
		RasterizeCircleOfRadius(fogOfWarComponent, modifiedRadius, offsets, nearObstacles, [fogOfWarComponent, nearObstacles, &components, &bottomTraces](const FogOfWarOffsets& offsets)
		{
			CircleQuadrant quadrant;
			quadrant.m_yValue = offsets.m_circleX <= offsets.m_bottomOffset ? offsets.m_circleX : offsets.m_bottomOffset;
			quadrant.m_xStartValue = offsets.m_circleY <= offsets.m_leftOffset ? offsets.m_circleY : offsets.m_leftOffset;
			quadrant.m_xEndValue = offsets.m_circleY <= offsets.m_rightOffset ? offsets.m_circleY : offsets.m_rightOffset;
			quadrant.m_centerColumnIndex = components.m_fogOfWarLocationComponent->m_fogOfWarPixel + (quadrant.m_yValue * static_cast<uint32>(fogOfWarComponent->m_textureSize));
			SetAlphaForCircleQuadrant(fogOfWarComponent, components, quadrant, nearObstacles, bottomTraces);
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

	TPair<int32, int32> points[3];
	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	GetPixelCoordsFromWorldSpaceLocation(fogOfWarComponent, spatialPartitioningComponent,  ArgusMath::ToUnrealVector2(point0), points[0]);
	GetPixelCoordsFromWorldSpaceLocation(fogOfWarComponent, spatialPartitioningComponent, ArgusMath::ToUnrealVector2(point1), points[1]);
	GetPixelCoordsFromWorldSpaceLocation(fogOfWarComponent, spatialPartitioningComponent, ArgusMath::ToUnrealVector2(point2), points[2]);

	auto CompareSwap = [](TPair<int32, int32>& pointA, TPair<int32, int32>& pointB) 
	{
		if (pointA.Value < pointB.Value || (pointA.Value == pointB.Value && pointA.Key > pointB.Key))
		{
			Swap(pointA, pointB);
		}
	};

	CompareSwap(points[0], points[1]);
	CompareSwap(points[1], points[2]);
	CompareSwap(points[0], points[1]);

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

	// Cache frequently accessed values
	const int32 textureSize = fogOfWarComponent->m_textureSize;
	uint8* textureData = fogOfWarComponent->m_textureData.GetData();
	
	// Use fixed-point arithmetic (16.16 format) to avoid float→int conversions
	const int32 deltaY = point1.Value - point0.Value; // Negative for flat-bottom
	const int32 fixedSlopeLeft = ((point1.Key - point0.Key) << 16) / deltaY;
	const int32 fixedSlopeRight = ((point2.Key - point0.Key) << 16) / deltaY;
	
	int32 fixedLeftX = point0.Key << 16;
	int32 fixedRightX = point0.Key << 16;
	
	for (int32 height = point0.Value; height >= point1.Value; --height)
	{
		const int32 heightIndex = height * textureSize;
		const int32 leftIndex = fixedLeftX >> 16;
		const int32 rightIndex = (fixedRightX + 0xFFFF) >> 16; // Equivalent to ceil
		
		// Inline memset - avoid function call overhead
		const int32 rowLength = (rightIndex - leftIndex) + 1;
		if (rowLength > 0)
		{
			memset(&textureData[heightIndex + leftIndex], 0, rowLength);
		}
		
		fixedLeftX -= fixedSlopeLeft;
		fixedRightX -= fixedSlopeRight;
	}
}

void FogOfWarSystems::FillFlatTopTriangle(FogOfWarComponent* fogOfWarComponent, const TPair<int32, int32>& point0, const TPair<int32, int32>& point1, const TPair<int32, int32>& point2)
{
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	if (point2.Value == point0.Value || point2.Value == point1.Value)
	{
		return;
	}

	// Cache frequently accessed values
	const int32 textureSize = fogOfWarComponent->m_textureSize;
	uint8* textureData = fogOfWarComponent->m_textureData.GetData();
	
	// Use fixed-point arithmetic (16.16 format)
	const int32 deltaY = point2.Value - point0.Value; // Negative for flat-top
	const int32 fixedSlopeLeft = ((point2.Key - point0.Key) << 16) / deltaY;
	const int32 fixedSlopeRight = ((point2.Key - point1.Key) << 16) / deltaY;
	
	int32 fixedLeftX = point2.Key << 16;
	int32 fixedRightX = point2.Key << 16;
	
	for (int32 height = point2.Value; height <= point0.Value; ++height)
	{
		const int32 heightIndex = height * textureSize;
		const int32 leftIndex = fixedLeftX >> 16;
		const int32 rightIndex = (fixedRightX + 0xFFFF) >> 16;
		
		const int32 rowLength = (rightIndex - leftIndex) + 1;
		if (rowLength > 0)
		{
			memset(&textureData[heightIndex + leftIndex], 0, rowLength);
		}
		
		fixedLeftX += fixedSlopeLeft;
		fixedRightX += fixedSlopeRight;
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

void FogOfWarSystems::RevealPixelRangeWithObstacles(FogOfWarComponent* fogOfWarComponent, const SpatialPartitioningComponent* spatialPartitioningComponent, uint32 fromPixelInclusive, uint32 toPixelInclusive, const ObstaclePointKDTreeRangeOutput& obstacleIndicies, const FVector2D& cartesianEntityLocation, FVector2D& prevFrom, FVector2D& prevTo)
{
	ARGUS_TRACE(FogOfWarSystems::RevealPixelRangeWithObstacles);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const float visionAdjustDistance = fogOfWarComponent->m_visionObstacleAdjustDistance;
	const FVector2D cartesianFromLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, spatialPartitioningComponent, fromPixelInclusive));
	const FVector2D cartesianToLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, spatialPartitioningComponent, toPixelInclusive));

	FVector2D currentFromIntersection = cartesianFromLocation;
	FVector2D currentToIntersection = cartesianToLocation;
	float currentFromDistanceSquared = FVector2D::DistSquared(cartesianEntityLocation, currentFromIntersection);
	float currentToDistanceSquared = FVector2D::DistSquared(cartesianEntityLocation, currentToIntersection);
	
	const TArray<ObstacleIndicies, ArgusContainerAllocator<20u> >& inRangeObstacleIndicies = obstacleIndicies.GetInRangeObstacleIndicies();
	for (int32 i = 0; i < inRangeObstacleIndicies.Num(); ++i)
	{
		const ObstacleIndicies& indexPair = inRangeObstacleIndicies[i];
		if (spatialPartitioningComponent->IsPointElevated(indexPair) || spatialPartitioningComponent->IsNextPointElevated(indexPair))
		{
			continue;
		}

		const ObstaclePoint& currentObstaclePoint = spatialPartitioningComponent->GetObstaclePointFromIndicies(indexPair);
		const ObstaclePoint& nextObstaclePoint = spatialPartitioningComponent->GetNextObstaclePointFromIndicies(indexPair);

		FVector2D currentPoint = currentObstaclePoint.m_point;
		const FVector2D currentLeft = currentObstaclePoint.GetLeftVector();
		FVector2D nextPoint = nextObstaclePoint.m_point;
		const FVector2D nextLeft = nextObstaclePoint.GetLeftVector();

		currentPoint += (currentLeft * visionAdjustDistance);
		nextPoint += (nextLeft * visionAdjustDistance);

		if (ArgusMath::IsLeftOfCartesian(currentPoint, nextPoint, cartesianEntityLocation))
		{
			continue;
		}

		FVector2D fromIntersection = cartesianFromLocation;
		FVector2D toIntersection = cartesianToLocation;

		if (ArgusMath::GetLineSegmentIntersectionCartesian(cartesianEntityLocation, cartesianFromLocation, currentPoint, nextPoint, fromIntersection))
		{
			const float fromDistanceSquared = FVector2D::DistSquared(cartesianEntityLocation, fromIntersection);
			if (fromDistanceSquared < currentFromDistanceSquared)
			{
				currentFromIntersection = fromIntersection;
				currentFromDistanceSquared = fromDistanceSquared;
			}
		}

		if (ArgusMath::GetLineSegmentIntersectionCartesian(cartesianEntityLocation, cartesianToLocation, currentPoint, nextPoint, toIntersection))
		{
			const float toDistanceSquared = FVector2D::DistSquared(cartesianEntityLocation, toIntersection);
			if (toDistanceSquared < currentToDistanceSquared)
			{
				currentToIntersection = toIntersection;
				currentToDistanceSquared = toDistanceSquared;
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

void FogOfWarSystems::SetAlphaForCircleQuadrant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const CircleQuadrant& quadrant, const bool hasObstacles, QuadrantObstacleTraces& quadrantTraces)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForCircleQuadrant);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (hasObstacles && components.m_nearbyObstaclesComponent)
	{
		const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
		const FVector2D cartesianCenterLocation = ArgusMath::ToCartesianVector2(GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, components.m_fogOfWarLocationComponent->m_fogOfWarPixel));
		RevealPixelRangeWithObstacles(fogOfWarComponent, spatialPartitioningComponent, quadrant.m_centerColumnIndex - quadrant.m_xStartValue, quadrant.m_centerColumnIndex + quadrant.m_xEndValue,
			components.m_nearbyObstaclesComponent->m_obstacleIndicies, cartesianCenterLocation, quadrantTraces.m_previousLeft, quadrantTraces.m_previousRight);

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

bool FogOfWarSystems::GetPixelCoordsFromWorldSpaceLocation(FogOfWarComponent* fogOfWarComponent, const SpatialPartitioningComponent* spatialPartitioningComponent, const FVector2D& worldSpaceLocation, TPair<int32, int32>& ouputPair)
{
	ARGUS_RETURN_ON_NULL_BOOL(fogOfWarComponent, ArgusECSLog);
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
	ARGUS_RETURN_ON_NULL_VALUE(fogOfWarComponent, ArgusECSLog, 0u);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, 0u);

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
	return GetWorldSpaceLocationFromPixelNumber(fogOfWarComponent, ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>(), pixelNumber);
}

FVector2D FogOfWarSystems::GetWorldSpaceLocationFromPixelNumber(FogOfWarComponent* fogOfWarComponent, const SpatialPartitioningComponent* spatialPartitioningComponent, uint32 pixelNumber)
{
	ARGUS_RETURN_ON_NULL_VALUE(fogOfWarComponent, ArgusECSLog, FVector2D::ZeroVector);
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, FVector2D::ZeroVector);

	const float worldspaceWidth = spatialPartitioningComponent->m_validSpaceExtent * 2.0f;
	const float textureIncrement = ArgusMath::SafeDivide(worldspaceWidth, static_cast<float>(fogOfWarComponent->m_textureSize));

	const float leftOffset = static_cast<float>(pixelNumber % fogOfWarComponent->m_textureSize) * textureIncrement;
	const float topOffset = static_cast<float>(pixelNumber / fogOfWarComponent->m_textureSize) * textureIncrement;
	
	FVector2D output = FVector2D(spatialPartitioningComponent->m_validSpaceExtent - topOffset, leftOffset - spatialPartitioningComponent->m_validSpaceExtent);
	ClampVectorToWorldBounds(output);
	return output;
}

uint32 FogOfWarSystems::GetPixelRadiusFromWorldSpaceRadius(FogOfWarComponent* fogOfWarComponent, float radius)
{
	ARGUS_RETURN_ON_NULL_VALUE(fogOfWarComponent, ArgusECSLog, 0u);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(spatialPartitioningComponent, ArgusECSLog, 0u);

	const float portion = ArgusMath::SafeDivide(radius, (2.0f * spatialPartitioningComponent->m_validSpaceExtent));
	return FMath::FloorToInt32(static_cast<float>(fogOfWarComponent->m_textureSize) * portion);
}

void FogOfWarSystems::ClampVectorToWorldBounds(FVector2D& vector)
{
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	vector.X = FMath::Clamp(vector.X, -spatialPartitioningComponent->m_validSpaceExtent, spatialPartitioningComponent->m_validSpaceExtent);
	vector.Y = FMath::Clamp(vector.Y, -spatialPartitioningComponent->m_validSpaceExtent, spatialPartitioningComponent->m_validSpaceExtent);
}

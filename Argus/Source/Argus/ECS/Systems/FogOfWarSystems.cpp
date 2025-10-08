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

	fogOfWarComponent->m_fogOfWarTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	fogOfWarComponent->m_fogOfWarTexture->SRGB = 0;
	fogOfWarComponent->m_fogOfWarTexture->Filter = TextureFilter::TF_Nearest;
	fogOfWarComponent->m_fogOfWarTexture->AddToRoot();
	fogOfWarComponent->m_fogOfWarTexture->UpdateResource();
	fogOfWarComponent->m_textureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, fogOfWarComponent->m_textureSize, fogOfWarComponent->m_textureSize);

	fogOfWarComponent->m_textureData.Init(255u, fogOfWarComponent->GetTotalPixels());
	fogOfWarComponent->m_smoothedTextureData.Init(255u, fogOfWarComponent->GetTotalPixels());
	fogOfWarComponent->m_intermediarySmoothingData.Init(255.0f, fogOfWarComponent->GetTotalPixels());

	if (fogOfWarComponent->m_useBlurring)
	{
		fogOfWarComponent->m_blurredTextureData.Init(255u, fogOfWarComponent->GetTotalPixels());
		InitializeGaussianFilter(fogOfWarComponent);
	}
}

void FogOfWarSystems::RunThreadSystems(float deltaTime)
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Iterate over all entities and carve out a circle of pixels (activelyRevealed) based on sight radius for entities that are on the local player team (or allies).
	SetRevealedStatePerEntity(fogOfWarComponent);

	// Iterate over all entities and apply gaussian filter.
	if (fogOfWarComponent->m_useBlurring)
	{
		ApplyGaussianBlur(fogOfWarComponent);
	}

	// Take our result target state and use exponential decay smoothing to get a final state.
	ApplyExponentialDecaySmoothing(fogOfWarComponent, deltaTime);
}

void FogOfWarSystems::RunSystems()
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	UpdateTexture();
	UpdateDynamicMaterialInstance();
}

void FogOfWarSystems::InitializeGaussianFilter(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Max filter size is 8 by 8 due to SIMD
	fogOfWarComponent->m_gaussianFilter.SetNumZeroed(64);

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
		}
		else if (components.m_fogOfWarLocationComponent->m_clearedThisFrame)
		{
			RevealPixelAlphaForEntity(fogOfWarComponent, components, offsets, false);
			components.m_fogOfWarLocationComponent->m_fogOfWarPixel = MAX_uint32;
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

		UpdateDoesEntityNeedToUpdateActivelyRevealed(components, inputInterfaceComponent);
		if (!components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame)
		{
			continue;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		RevealPixelAlphaForEntity(fogOfWarComponent, components, offsets, true);
	}
}

void FogOfWarSystems::ApplyGaussianBlur(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_TRACE(FogOfWarSystems::ApplyGaussianBlur);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Do Gaussian Blur per entity.
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(entity))
		{
			continue;
		}

		if (!components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame)
		{
			continue;
		}

		FogOfWarOffsets offsets;
		PopulateOffsetsForEntity(fogOfWarComponent, components, offsets);
		BlurBoundariesForEntity(fogOfWarComponent, components, offsets);
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
	const uint8* sourceArray = fogOfWarComponent->m_useBlurring ? fogOfWarComponent->m_blurredTextureData.GetData() : fogOfWarComponent->m_textureData.GetData();
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

	const uint32 radius = GetPixelRadiusFromWorldSpaceRadius(fogOfWarComponent, components.m_targetingComponent->m_sightRange);
	RasterizeCircleOfRadius(radius, offsets, [fogOfWarComponent, &components, activelyRevealed](const FogOfWarOffsets& offsets)
	{
		// Set Alpha for pixel range for all symmetrical pixels.
		SetAlphaForCircleOctant(fogOfWarComponent, components, offsets, activelyRevealed);
	});
}

void FogOfWarSystems::BlurBoundariesForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, FogOfWarOffsets& offsets)
{
	ARGUS_TRACE(FogOfWarSystems::BlurBoundariesForEntity);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const uint32 radius = GetPixelRadiusFromWorldSpaceRadius(fogOfWarComponent, components.m_targetingComponent->m_sightRange);
	const uint32 innerRadius = radius - fogOfWarComponent->m_blurPassCount;
	TArray<int32> innerOctantXValues;
	innerOctantXValues.Reserve(innerRadius);
	RasterizeCircleOfRadius(innerRadius, offsets, [fogOfWarComponent, &components, &innerOctantXValues](FogOfWarOffsets& offsets)
	{
		innerOctantXValues.Add(offsets.m_circleX);
	});

	RasterizeCircleOfRadius(radius, offsets, [fogOfWarComponent, &components, &innerOctantXValues](FogOfWarOffsets& offsets)
	{
		if (static_cast<int32>(offsets.m_circleY) < innerOctantXValues.Num())
		{
			offsets.m_innerCircleX = innerOctantXValues[offsets.m_circleY];
		}
		else
		{
			offsets.m_innerCircleX = offsets.m_circleY - 1;
		}

		BlurBoundariesForCircleOctant(fogOfWarComponent, components, offsets);
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

	uint8* firstAddress = &fogOfWarComponent->m_textureData[fromPixelInclusive];
	uint32 rowLength = (toPixelInclusive - fromPixelInclusive) + 1;
	memset(firstAddress, activelyRevealed ? 0 : fogOfWarComponent->m_revealedOnceAlpha, rowLength);

	if (fogOfWarComponent->m_useBlurring)
	{
		uint8* secondAddress = &fogOfWarComponent->m_blurredTextureData[fromPixelInclusive];
		memset(secondAddress, activelyRevealed ? 0 : fogOfWarComponent->m_revealedOnceAlpha, rowLength);
	}
}

void FogOfWarSystems::SetAlphaForCircleOctant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::SetAlphaForCircleOctant);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	CircleOctantExpansion octantExpansion;
	PopulateOctantExpansionForEntity(fogOfWarComponent, components, offsets, octantExpansion);

	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnTopIndex - octantExpansion.m_topStartX, octantExpansion.m_centerColumnTopIndex + octantExpansion.m_topEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnMidUpIndex - octantExpansion.m_midUpStartX, octantExpansion.m_centerColumnMidUpIndex + octantExpansion.m_midUpEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnMidDownIndex - octantExpansion.m_midDownStartX, octantExpansion.m_centerColumnMidDownIndex + octantExpansion.m_midDownEndX, activelyRevealed);
	SetAlphaForPixelRange(fogOfWarComponent, octantExpansion.m_centerColumnBottomIndex - octantExpansion.m_bottomStartX, octantExpansion.m_centerColumnBottomIndex + octantExpansion.m_bottomEndX, activelyRevealed);
}

void FogOfWarSystems::BlurBoundariesForCircleOctant(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets)
{
	ARGUS_TRACE(FogOfWarSystems::BlurBoundariesForCircleOctant);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	CircleOctantExpansion octantExpansion;
	PopulateOctantExpansionForEntity(fogOfWarComponent, components, offsets, octantExpansion);

	int32 blurDistance = offsets.m_circleX - offsets.m_innerCircleX;
	for (int32 i = 0; i < blurDistance; ++i)
	{
		const int32 shiftedX = offsets.m_circleX - i;
		BlurAroundPixel(static_cast<int32>(offsets.m_circleY), static_cast<int32>(shiftedX), fogOfWarComponent, components);
		BlurAroundPixel(-static_cast<int32>(offsets.m_circleY), static_cast<int32>(shiftedX), fogOfWarComponent, components);
		BlurAroundPixel(static_cast<int32>(shiftedX), static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
		BlurAroundPixel(-static_cast<int32>(shiftedX), static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
		BlurAroundPixel(static_cast<int32>(shiftedX), -static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
		BlurAroundPixel(-static_cast<int32>(shiftedX), -static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
		BlurAroundPixel(static_cast<int32>(offsets.m_circleY), -static_cast<int32>(shiftedX), fogOfWarComponent, components);
		BlurAroundPixel(-static_cast<int32>(offsets.m_circleY), -static_cast<int32>(shiftedX), fogOfWarComponent, components);
	}
}

void FogOfWarSystems::SetRingAlphaForCircleOctant(uint8 alphaValue, FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, const FogOfWarOffsets& offsets)
{
	ARGUS_TRACE(FogOfWarSystems::BlurBoundariesForCircleOctant);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	CircleOctantExpansion octantExpansion;
	PopulateOctantExpansionForEntity(fogOfWarComponent, components, offsets, octantExpansion);

	int32 blurDistance = offsets.m_circleX - offsets.m_innerCircleX;
	for (int32 i = 0; i < blurDistance; ++i)
	{
		const int32 shiftedX = offsets.m_circleX - i;
		SetPixelAlpha(static_cast<int32>(offsets.m_circleY), static_cast<int32>(shiftedX), alphaValue, fogOfWarComponent, components);
		SetPixelAlpha(-static_cast<int32>(offsets.m_circleY), static_cast<int32>(shiftedX), alphaValue, fogOfWarComponent, components);
		SetPixelAlpha(static_cast<int32>(shiftedX), static_cast<int32>(offsets.m_circleY), alphaValue, fogOfWarComponent, components);
		SetPixelAlpha(-static_cast<int32>(shiftedX), static_cast<int32>(offsets.m_circleY), alphaValue, fogOfWarComponent, components);
		SetPixelAlpha(static_cast<int32>(shiftedX), -static_cast<int32>(offsets.m_circleY), alphaValue, fogOfWarComponent, components);
		SetPixelAlpha(-static_cast<int32>(shiftedX), -static_cast<int32>(offsets.m_circleY), alphaValue, fogOfWarComponent, components);
		SetPixelAlpha(static_cast<int32>(offsets.m_circleY), -static_cast<int32>(shiftedX), alphaValue, fogOfWarComponent, components);
		SetPixelAlpha(-static_cast<int32>(offsets.m_circleY), -static_cast<int32>(shiftedX), alphaValue, fogOfWarComponent, components);
	}
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
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcData = fogOfWarComponent->m_smoothedTextureData.GetData();

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

void FogOfWarSystems::UpdateDynamicMaterialInstance()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateDynamicMaterialInstance);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Set the dynamic material instance texture param to the fog of war texture.
	if (fogOfWarComponent->m_dynamicMaterialInstance && fogOfWarComponent->m_fogOfWarTexture)
	{
		fogOfWarComponent->m_dynamicMaterialInstance->SetTextureParameterValue("DynamicTexture", fogOfWarComponent->m_fogOfWarTexture);
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

	if (const VelocityComponent* velocityComponent = components.m_entity.GetComponent<VelocityComponent>())
	{
		if (!velocityComponent->m_currentVelocity.IsNearlyZero() || !velocityComponent->m_proposedAvoidanceVelocity.IsNearlyZero())
		{
			components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = true;
			return;
		}
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

	uint16 nearestMovingTeammateId = spatialPartitioningComponent->m_argusEntityKDTree.FindOtherArgusEntityIdClosestToArgusEntity(components.m_entity, queryFilter);
	if (nearestMovingTeammateId == ArgusECSConstants::k_maxEntities)
	{
		components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = false;
		return;
	}

	FogOfWarSystemsArgs otherComponents;
	if (!otherComponents.PopulateArguments(ArgusEntity::RetrieveEntity(nearestMovingTeammateId)))
	{
		components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = false;
		return;
	}

	const float distSquared = FVector::DistSquared(components.m_transformComponent->m_location, otherComponents.m_transformComponent->m_location);
	const float radiusSquared = FMath::Square(components.m_targetingComponent->m_sightRange + otherComponents.m_targetingComponent->m_sightRange);

	components.m_fogOfWarLocationComponent->m_needsActivelyRevealedThisFrame = distSquared < radiusSquared;
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

void FogOfWarSystems::BlurAroundPixel(int32 relativeX, int32 relativeY, FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components)
{
	ARGUS_TRACE(FogOfWarSystems::BlurAroundPixel);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (UNLIKELY(!components.AreComponentsValidCheck(ARGUS_FUNCNAME)))
	{
		return;
	}

	// TODO JAMES: Our in bounds checks are far and away the most expensive component of this function. Maybe we handle bounds checking differently?
	//if (!IsPixelInFogOfWarBounds(relativeX, relativeY, fogOfWarComponent, components))
	//{
	//	return;
	//}

	const uint8 radius = fogOfWarComponent->m_gaussianDimension / 2;

	// __m256 horizontalSum = _mm256_set1_ps(0.0f);
	float sum = 0.0f;
	for (int32 i = 0; i < fogOfWarComponent->m_gaussianDimension; ++i)
	{
		// Ok, so I tried SIMD here and it was essentially just less performant than my naive approach :(
		// We may just need to do this on the GPU to be more performant.
		// Leaving my attempt here for posterity.

		//const int32 initialRelX = relativeX - radius;
		//const int32 yOffset = (relativeY + (radius - i)) * static_cast<int32>(fogOfWarComponent->m_textureSize);
		//const int32 yLocation = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - yOffset;
		//__m256 textureFloats = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i*>(&fogOfWarComponent->m_textureData[yLocation + initialRelX]))));
		// __m256 filterFloats = _mm256_load_ps(&fogOfWarComponent->m_gaussianFilter[i * 8]);
		// horizontalSum = _mm256_add_ps(horizontalSum, _mm256_mul_ps(textureFloats, filterFloats));

		for (int32 j = 0; j < fogOfWarComponent->m_gaussianDimension; ++j)
		{
			const int32 shiftedX = relativeX + (j - radius);
			const int32 shiftedY = relativeY + (radius - i);
			// TODO JAMES: Our in bounds checks are far and away the most expensive component of this function. Maybe we handle bounds checking differently?
			//if (!IsPixelInFogOfWarBounds(shiftedX, shiftedY, fogOfWarComponent, components))
			//{
			//	continue;
			//}
			const int32 yOffset = shiftedY * static_cast<int32>(fogOfWarComponent->m_textureSize);
			const int32 yLocation = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - yOffset;
			const uint8 pixelValue = fogOfWarComponent->m_textureData[yLocation + shiftedX];
			sum += (static_cast<float>(pixelValue) * fogOfWarComponent->m_gaussianFilter[(i * fogOfWarComponent->m_gaussianDimension) + j]);
		}
	}

	const int32 yOffset = relativeY * static_cast<int32>(fogOfWarComponent->m_textureSize);
	const int32 yLocation = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - yOffset;
	fogOfWarComponent->m_blurredTextureData[yLocation + relativeX] = static_cast<uint8>(FMath::FloorToInt(sum));
}

void FogOfWarSystems::SetPixelAlpha(int32 relativeX, int32 relativeY, uint8 alphaValue, FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components)
{
	ARGUS_TRACE(FogOfWarSystems::SetPixelAlpha);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const int32 yOffset = relativeY * static_cast<int32>(fogOfWarComponent->m_textureSize);
	const int32 yLocation = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - yOffset;
	fogOfWarComponent->m_blurredTextureData[yLocation + relativeX] = alphaValue;
}

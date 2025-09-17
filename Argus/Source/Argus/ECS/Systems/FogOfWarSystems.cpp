// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FogOfWarSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "SystemArgumentDefinitions/FogOfWarSystemsArgs.h"

const float FogOfWarSystems::k_gaussianFilter[9] =
{
	0.01f, 0.08f, 0.01f,
	0.08f, 0.64f, 0.08f,
	0.01f, 0.08f, 0.01f
};

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

	fogOfWarComponent->m_textureData.SetNumZeroed(fogOfWarComponent->GetTotalPixels());
	fogOfWarComponent->m_blurredTextureData.SetNumZeroed(fogOfWarComponent->GetTotalPixels());
	memset(fogOfWarComponent->m_textureData.GetData(), 255, fogOfWarComponent->GetTotalPixels());
	memset(fogOfWarComponent->m_blurredTextureData.GetData(), 255, fogOfWarComponent->GetTotalPixels());
	InitializeGaussianFilter(fogOfWarComponent);
}

void FogOfWarSystems::RunThreadSystems()
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Iterate over all entities and carve out a circle of pixels (activelyRevealed) based on sight radius for entities that are on the local player team (or allies).
	SetRevealedStatePerEntity(fogOfWarComponent);
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

	const uint8 filterSize = fogOfWarComponent->m_gaussianFilterDimension * fogOfWarComponent->m_gaussianFilterDimension;
	fogOfWarComponent->m_gaussianFilter.SetNumZeroed(filterSize);

	const float radius = static_cast<float>(fogOfWarComponent->m_gaussianFilterDimension / 2);
	const float radiusSquaredReciprocal = 1.0f / (2.0f * FMath::Square(radius));
	const float squareRootPiReciprocal = 1.0f / (FMath::Sqrt(UE_TWO_PI) * radius);

	float shiftedRadius = -radius;
	float sum = 0.0f;
	for (uint8 i = 0; i < fogOfWarComponent->m_gaussianFilterDimension; ++i)
	{
		const float squareShiftedRadius = FMath::Square(shiftedRadius);
		fogOfWarComponent->m_gaussianFilter[i] = squareRootPiReciprocal * FMath::Exp(-squareShiftedRadius * radiusSquaredReciprocal);
		sum += fogOfWarComponent->m_gaussianFilter[i];
		shiftedRadius += 1.0f;
	}

	if (FMath::IsNearlyZero(sum))
	{
		return;
	}

	for (uint8 i = 0; i < fogOfWarComponent->m_gaussianFilterDimension; ++i)
	{
		fogOfWarComponent->m_gaussianFilter[i] /= sum;
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
	RasterizeCircleOfRadius(radius, offsets, [fogOfWarComponent, &components](const FogOfWarOffsets& offsets)
	{
		// Gaussian Blur pass 1.
		BlurBoundariesForCircleOctant(fogOfWarComponent, components, offsets);
	});

	// TODO JAMES: We likely need to do a memset of the first pass bluring into the base for our second blur pass.

	RasterizeCircleOfRadius(radius - 1, offsets, [fogOfWarComponent, &components](const FogOfWarOffsets& offsets)
	{
		// Gaussian Blur pass 2.
		BlurBoundariesForCircleOctant(fogOfWarComponent, components, offsets);
	});
}

void FogOfWarSystems::RasterizeCircleOfRadius(uint32 radius, FogOfWarOffsets& offsets, TFunction<void(const FogOfWarOffsets& offsets)> perOctantPixelFunction)
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
	uint8* secondAddress = &fogOfWarComponent->m_blurredTextureData[fromPixelInclusive];
	uint32 rowLength = (toPixelInclusive - fromPixelInclusive) + 1;
	memset(firstAddress, activelyRevealed ? 0 : fogOfWarComponent->m_revealedOnceAlpha, rowLength);
	memset(secondAddress, activelyRevealed ? 0 : fogOfWarComponent->m_revealedOnceAlpha, rowLength);
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

	BlurAroundPixel(static_cast<int32>(offsets.m_circleY), static_cast<int32>(offsets.m_circleX), fogOfWarComponent, components);
	BlurAroundPixel(-static_cast<int32>(offsets.m_circleY), static_cast<int32>(offsets.m_circleX), fogOfWarComponent, components);
	BlurAroundPixel(static_cast<int32>(offsets.m_circleX), static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
	BlurAroundPixel(-static_cast<int32>(offsets.m_circleX), static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
	BlurAroundPixel(static_cast<int32>(offsets.m_circleX), -static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
	BlurAroundPixel(-static_cast<int32>(offsets.m_circleX), -static_cast<int32>(offsets.m_circleY), fogOfWarComponent, components);
	BlurAroundPixel(static_cast<int32>(offsets.m_circleY), -static_cast<int32>(offsets.m_circleX), fogOfWarComponent, components);
	BlurAroundPixel(-static_cast<int32>(offsets.m_circleY), -static_cast<int32>(offsets.m_circleX), fogOfWarComponent, components);
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
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcData = fogOfWarComponent->m_blurredTextureData.GetData();

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
	ARGUS_RETURN_ON_NULL_UINT32(fogOfWarComponent, ArgusECSLog);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_UINT32(spatialPartitioningComponent, ArgusECSLog);

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
	ARGUS_RETURN_ON_NULL_UINT32(fogOfWarComponent, ArgusECSLog);
	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL_UINT32(spatialPartitioningComponent, ArgusECSLog);

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

// TODO JAMES: At current texture sizes, even without the actual summing and setting of alpha, this is really brutal performance. Like 4ms a frame performance.
void FogOfWarSystems::BlurAroundPixel(int32 relativeX, int32 relativeY, FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components)
{
	ARGUS_TRACE(FogOfWarSystems::BlurAroundPixel);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!IsPixelInFogOfWarBounds(relativeX, relativeY, fogOfWarComponent, components))
	{
		return;
	}

	float sum = 0.0f;
	for (int32 i = 0; i < 3; ++i)
	{
		for (int32 j = 0; j < 3; ++j)
		{
			const int32 shiftedX = relativeX + (j - 1);
			const int32 shiftedY = relativeY + (1 - i);
			if (!IsPixelInFogOfWarBounds(shiftedX, shiftedY, fogOfWarComponent, components))
			{
				continue;
			}
			const int32 yOffset = shiftedY * static_cast<int32>(fogOfWarComponent->m_textureSize);
			const int32 yLocation = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - yOffset;
			const uint8 pixelValue = fogOfWarComponent->m_textureData[yLocation + shiftedX];
			sum += (static_cast<float>(pixelValue) * k_gaussianFilter[(i * 3) + j]);
		}
	}

	const int32 yOffset = relativeY * static_cast<int32>(fogOfWarComponent->m_textureSize);
	const int32 yLocation = components.m_fogOfWarLocationComponent->m_fogOfWarPixel - yOffset;
	fogOfWarComponent->m_blurredTextureData[yLocation + relativeX] = static_cast<uint8>(FMath::FloorToInt(sum));
}

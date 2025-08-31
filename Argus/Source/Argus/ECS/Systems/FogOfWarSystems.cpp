// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FogOfWarSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "SystemArgumentDefinitions/FogOfWarSystemsArgs.h"

void FogOfWarSystems::InitializeSystems()
{
	ARGUS_TRACE(FogOfWarSystems::InitializeSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	fogOfWarComponent->m_fogOfWarTexture = UTexture2D::CreateTransient(fogOfWarComponent->m_textureSize, fogOfWarComponent->m_textureSize);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_fogOfWarTexture, ArgusECSLog);

	fogOfWarComponent->m_fogOfWarTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	fogOfWarComponent->m_fogOfWarTexture->SRGB = 0;
	fogOfWarComponent->m_fogOfWarTexture->Filter = TextureFilter::TF_Nearest;
	fogOfWarComponent->m_fogOfWarTexture->AddToRoot();
	fogOfWarComponent->m_fogOfWarTexture->UpdateResource();
	fogOfWarComponent->m_textureRegion = FUpdateTextureRegion2D(0, 0, 0, 0, fogOfWarComponent->m_textureSize, fogOfWarComponent->m_textureSize);

	// TODO JAMES: Just hackily using FMemory for now. There will be additional complexity in making this properly resettable with the ArgusMemorySource
	const uint32 totalIndicies = fogOfWarComponent->GetTotalPixels() * 4u;
	fogOfWarComponent->m_textureData.SetNumZeroed(totalIndicies * 4u);
	
	for (uint32 i = 3; i < totalIndicies; i += 4)
	{
		fogOfWarComponent->m_textureData[i] = 255u;	// A
	}
}

void FogOfWarSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// Iterate over all entities and carve out a circle of pixels (activelyRevealed) based on sight radius for entities that are on the local player team (or allies).
	SetRevealedStatePerEntity(fogOfWarComponent);

	// Update the fog of war texture via render command
	UpdateTexture();

	// Set the dynamic material instance texture param to the fog of war texture.
	if (fogOfWarComponent->m_dynamicMaterialInstance)
	{
		ARGUS_TRACE(FogOfWarSystems::SetTextureParameterValue);
		fogOfWarComponent->m_dynamicMaterialInstance->SetTextureParameterValue("DynamicTexture", fogOfWarComponent->m_fogOfWarTexture);
	}
}

void FogOfWarSystems::SetRevealedStatePerEntity(FogOfWarComponent* fogOfWarComponent)
{
	ARGUS_TRACE(FogOfWarSystems::SetRevealedPixels);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		const ArgusEntity entity = ArgusEntity::RetrieveEntity(i);
		FogOfWarSystemsArgs components;
		if (!components.PopulateArguments(entity))
		{
			continue;
		}

		// TODO JAMES: No hardcode pls
		if (!entity.IsOnTeam(ETeam::TeamA))
		{
			continue;
		}

		const uint32 centerPixel = GetPixelNumberFromWorldSpaceLocation(fogOfWarComponent, components.m_transformComponent->m_location);
		if (((centerPixel != components.m_transformComponent->m_fogOfWarPixel) || !entity.IsAlive()) && components.m_transformComponent->m_fogOfWarPixel != MAX_uint32)
		{
			RevealPixelAlphaForEntity(fogOfWarComponent, components, false);
		}

		if (!entity.IsAlive())
		{
			components.m_transformComponent->m_fogOfWarPixel = MAX_uint32;
			continue;
		}
		components.m_transformComponent->m_fogOfWarPixel = centerPixel;
		
		RevealPixelAlphaForEntity(fogOfWarComponent, components, true);
	}
}

void FogOfWarSystems::RevealPixelAlphaForEntity(FogOfWarComponent* fogOfWarComponent, const FogOfWarSystemsArgs& components, bool activelyRevealed)
{
	ARGUS_TRACE(FogOfWarSystems::RevealPixelsForEntity);
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
	const uint32 centerPixelRowNumber = components.m_transformComponent->m_fogOfWarPixel / textureSize;

	uint32 leftOffset = radius;
	// The radius overlaps the left edge of the texture.
	if (((components.m_transformComponent->m_fogOfWarPixel - leftOffset) / textureSize) != centerPixelRowNumber)
	{
		leftOffset = components.m_transformComponent->m_fogOfWarPixel % textureSize;
	}

	uint32 rightOffset = radius;
	// The radius overlaps the right edge of the texture.
	if (((components.m_transformComponent->m_fogOfWarPixel + rightOffset) / textureSize) != centerPixelRowNumber)
	{
		rightOffset = textureSize - (components.m_transformComponent->m_fogOfWarPixel % textureSize);
	}

	uint32 topOffset = radius;
	// The radius overlaps the top edge of the texture.
	if (topOffset > centerPixelRowNumber)
	{
		topOffset = centerPixelRowNumber;
	}

	uint32 bottomOffset = radius;
	// The radius overlaps the bottom edge of the texture.
	if ((bottomOffset + centerPixelRowNumber) > textureSize)
	{
		bottomOffset = ((maxPixel - (textureSize - (components.m_transformComponent->m_fogOfWarPixel % textureSize))) / textureSize) - centerPixelRowNumber;
	}


	const uint32 topLeftIndex = (components.m_transformComponent->m_fogOfWarPixel - (topOffset * textureSize)) - leftOffset;
	if (activelyRevealed)
	{
		for (uint32 rowNumber = 0u; rowNumber <= (topOffset + bottomOffset); ++rowNumber)
		{
			uint32 pixelStart = topLeftIndex + (rowNumber * textureSize);
			uint8* firstAddress = &fogOfWarComponent->m_textureData[(pixelStart * 4u)];
			uint32 rowLength = (leftOffset + rightOffset) * 4u;
			memset(firstAddress, 0, rowLength);
		}
	}
	else
	{
		for (uint32 rowNumber = 0u; rowNumber <= (topOffset + bottomOffset); ++rowNumber)
		{
			uint32 pixelStart = topLeftIndex + (rowNumber * textureSize);
			for (uint32 columnNumber = 0u; columnNumber <= (leftOffset + rightOffset); ++columnNumber)
			{
				fogOfWarComponent->m_textureData[((pixelStart + columnNumber) * 4u) + 3] = fogOfWarComponent->m_revealedOnceAlpha;
			}
		}
	}
}

void FogOfWarSystems::UpdateTexture()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateTexture);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_fogOfWarTexture, ArgusECSLog);

	fogOfWarComponent->m_textureRegionsUpdateData = TextureRegionsUpdateData();

	fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource = reinterpret_cast<FTexture2DResource*>(fogOfWarComponent->m_fogOfWarTexture->GetResource());
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource, ArgusECSLog);

	fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI = fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource->GetTexture2DRHI();
	fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex = 0;
	fogOfWarComponent->m_textureRegionsUpdateData.m_numRegions = 1;
	fogOfWarComponent->m_textureRegionsUpdateData.m_regions = &fogOfWarComponent->m_textureRegion;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch = 4 * fogOfWarComponent->m_textureSize;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcBpp = 4;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcData = fogOfWarComponent->m_textureData.GetData();

	if (!fogOfWarComponent->m_textureRegionsUpdateData.m_srcData)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
		[fogOfWarComponent](FRHICommandListImmediate& RHICmdList)
		{
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

bool FogOfWarSystems::DoesPixelEqualColor(FogOfWarComponent* fogOfWarComponent, uint32 pixelNumber, FColor color)
{
	ARGUS_RETURN_ON_NULL_BOOL(fogOfWarComponent, ArgusECSLog);

	const uint32 totalPixels = fogOfWarComponent->GetTotalPixels();
	if (pixelNumber > (static_cast<uint32>(fogOfWarComponent->m_textureData.Num()) / 4u))
	{
		return false;
	}

	return	fogOfWarComponent->m_textureData[(pixelNumber * 4)] == color.B &&		// B
			fogOfWarComponent->m_textureData[(pixelNumber * 4) + 1] == color.G &&	// G
			fogOfWarComponent->m_textureData[(pixelNumber * 4) + 2] == color.R &&	// R
			fogOfWarComponent->m_textureData[(pixelNumber * 4) + 3] == color.A;		// A
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

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FogOfWarSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"

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
	fogOfWarComponent->m_textureData = new uint8[FMath::Square(fogOfWarComponent->m_textureSize) * 4u];
	
}

void FogOfWarSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(FogOfWarSystems::RunSystems);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);

	// TODO JAMES: Do stuff lmao

	UpdateTexture();
}

void FogOfWarSystems::UpdateTexture()
{
	ARGUS_TRACE(FogOfWarSystems::UpdateTexture);

	FogOfWarComponent* fogOfWarComponent = ArgusEntity::RetrieveEntity(ArgusECSConstants::k_singletonEntityId).GetComponent<FogOfWarComponent>();
	ARGUS_RETURN_ON_NULL(fogOfWarComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_fogOfWarTexture, ArgusECSLog);
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_textureData, ArgusECSLog);

	fogOfWarComponent->m_textureRegionsUpdateData = TextureRegionsUpdateData();

	fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource = reinterpret_cast<FTexture2DResource*>(fogOfWarComponent->m_fogOfWarTexture->GetResource());
	ARGUS_RETURN_ON_NULL(fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource, ArgusECSLog);

	fogOfWarComponent->m_textureRegionsUpdateData.m_textureRHI = fogOfWarComponent->m_textureRegionsUpdateData.m_texture2DResource->GetTexture2DRHI();
	fogOfWarComponent->m_textureRegionsUpdateData.m_mipIndex = 0;
	fogOfWarComponent->m_textureRegionsUpdateData.m_numRegions = 1;
	fogOfWarComponent->m_textureRegionsUpdateData.m_regions = &fogOfWarComponent->m_textureRegion;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcPitch = 4 * fogOfWarComponent->m_textureSize;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcBpp = 4;
	fogOfWarComponent->m_textureRegionsUpdateData.m_srcData = fogOfWarComponent->m_textureData;

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

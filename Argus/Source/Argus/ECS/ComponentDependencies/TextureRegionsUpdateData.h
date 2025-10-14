// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

struct TextureRegionsUpdateData
{
	FTexture2DResource* m_texture2DResource;
	FRHITexture* m_textureRHI;
	int32 m_mipIndex;
	uint32 m_numRegions;
	FUpdateTextureRegion2D* m_regions;
	uint32 m_srcPitch;
	uint32 m_srcBpp;
	uint8* m_srcData;
};

struct TextureRegionsFloatUpdateData
{
	FTexture2DResource* m_texture2DResource;
	FRHITexture* m_textureRHI;
	int32 m_mipIndex;
	uint32 m_numRegions;
	FUpdateTextureRegion2D* m_regions;
	uint32 m_srcPitch;
	uint32 m_srcBpp;
	float* m_srcData;
};
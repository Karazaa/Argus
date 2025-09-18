// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/TextureRegionsUpdateData.h"
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "RHITypes.h"

struct FogOfWarComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_PROPERTY(Transient)
	TObjectPtr<UTexture2D> m_fogOfWarTexture = nullptr;

	ARGUS_PROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> m_dynamicMaterialInstance = nullptr;

	ARGUS_IGNORE()
	FUpdateTextureRegion2D m_textureRegion;

	ARGUS_IGNORE()
	TextureRegionsUpdateData m_textureRegionsUpdateData;

	ARGUS_IGNORE()
	TArray<uint8, ArgusContainerAllocator<0> > m_textureData;

	ARGUS_IGNORE()
	TArray<uint8, ArgusContainerAllocator<0> > m_blurredTextureData;

	ARGUS_IGNORE()
	TArray<float, ArgusContainerAllocator<0> > m_gaussianFilter;

	uint8 m_gaussianDimension = 5u;
	uint8 m_revealedOnceAlpha = 100u;
	uint8 m_blurPassCount = 3u;
	bool m_useBlurring = true;
	uint16 m_textureSize = 1024u;

	uint32 GetTotalPixels()
	{
		return static_cast<uint32>(m_textureSize) * static_cast<uint32>(m_textureSize);
	}

	void CopyBlurredDataIntoTextureData()
	{
		ARGUS_TRACE(FogOfWarComponent::CopyBlurredDataIntoTextureData);

		uint8* textureAddress = &m_textureData[0];
		uint8* blurredAddress = &m_blurredTextureData[0];
		memcpy(textureAddress, blurredAddress, m_textureData.Num());
	}
};

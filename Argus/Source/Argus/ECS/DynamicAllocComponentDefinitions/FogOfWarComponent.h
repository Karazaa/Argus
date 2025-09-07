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

	uint8 m_revealedOnceAlpha = 100u;
	uint16 m_textureSize = 1024u;

	uint32 GetTotalPixels()
	{
		return static_cast<uint32>(m_textureSize) * static_cast<uint32>(m_textureSize);
	}
};

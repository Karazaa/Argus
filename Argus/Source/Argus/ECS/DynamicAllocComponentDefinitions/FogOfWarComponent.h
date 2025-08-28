// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

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

	// TODO JAMES: need to sort out dynamic allocation (with the resetting paradigm that comes from dynamic alloc components) 
	ARGUS_IGNORE()
	TArray<uint8> m_textureData;

	FColor m_hiddenColor;
	FColor m_revealedOnceColor;
	FColor m_activelyRevealedColor;
	uint16 m_textureSize = 1024u;

	uint32 GetTotalPixels()
	{
		return static_cast<uint32>(m_textureSize) * static_cast<uint32>(m_textureSize);
	}
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/TextureRegionsUpdateData.h"
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "RHITypes.h"

struct FogOfWarComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_PROPERTY(Transient)
	TObjectPtr<UTexture2D> m_fogOfWarTexture = nullptr;

	//ARGUS_PROPERTY(Transient)
	//TObjectPtr<UTexture2D> m_previousFogOfwarTexture = nullptr;

	ARGUS_IGNORE()
	FUpdateTextureRegion2D m_textureRegion;

	ARGUS_IGNORE()
	TextureRegionsUpdateData m_textureRegionsUpdateData;

	// TODO JAMES: need to sort out dynamic allocation (with the resetting paradigm that comes from dynamic alloc components) 
	ARGUS_IGNORE()
	uint8* m_textureData = nullptr;

	uint16 m_textureSize = 1024u;

};

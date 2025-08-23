// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"

struct FogOfWarComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	uint16 m_textureSize = 1024u;

	ARGUS_PROPERTY(Transient)
	TObjectPtr<UTexture2D> m_fogOfWarTexture = nullptr;

	ARGUS_PROPERTY(Transient)
	TObjectPtr<UTexture2D> m_previousFogOfwarTexture = nullptr;
};

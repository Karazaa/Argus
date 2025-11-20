// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct FogOfWarLocationComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	uint32 m_fogOfWarPixel = MAX_uint32;
};

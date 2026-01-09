// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "SoftPtrLoadStore.h"
#include "CoreMinimal.h"

struct MaterialCacheComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	FSoftObjectLoadStore_UMaterialInterface m_moveToLocationDecalMaterial;

	FSoftObjectLoadStore_UMaterialInterface m_attackMoveToLocationDecalMaterial;
};

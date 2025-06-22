// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"

struct AssetLoadingComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_IGNORE()
	FStreamableManager m_streamableManager;

	AssetLoadingComponent() {}
	AssetLoadingComponent(const AssetLoadingComponent&) {}
	AssetLoadingComponent& operator=(const AssetLoadingComponent&) { return *this; }
};

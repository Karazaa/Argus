// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

class UWorld;

struct WorldReferenceComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_IGNORE()
	UWorld* m_worldPointer = nullptr;
};

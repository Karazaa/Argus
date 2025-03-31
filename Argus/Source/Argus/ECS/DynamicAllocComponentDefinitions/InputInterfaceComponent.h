// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct InputInterfaceComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED

	ARGUS_IGNORE()
	TArray<uint16> m_selectedArgusEntityIds;

	ARGUS_IGNORE()
	TArray<uint16> m_activeAbilityGroupArgusEntityIds;
};
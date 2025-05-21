// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

UENUM()
enum class ESelectedActorsDisplayState : uint8
{
	NotChanged,
	ChangedThisFrame
};

struct InputInterfaceComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED

	ARGUS_IGNORE()
	TArray<uint16> m_selectedArgusEntityIds;

	ARGUS_IGNORE()
	TArray<uint16> m_activeAbilityGroupArgusEntityIds;

	ARGUS_IGNORE()
	ESelectedActorsDisplayState m_selectedActorsDisplayState = ESelectedActorsDisplayState::NotChanged;
};
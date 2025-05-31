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
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	ARGUS_IGNORE()
	TArray<uint16> m_selectedArgusEntityIds;

	ARGUS_IGNORE()
	TArray<uint16> m_activeAbilityGroupArgusEntityIds;

	ARGUS_IGNORE()
	ESelectedActorsDisplayState m_selectedActorsDisplayState = ESelectedActorsDisplayState::NotChanged;

	ARGUS_IGNORE()
	int8 m_indexOfActiveAbilityGroup = 0;

	bool IsEntityIdInActiveAbilityGroup(uint16 entityId) const
	{
		for (int32 i = 0; i < m_activeAbilityGroupArgusEntityIds.Num(); ++i)
		{
			if (m_activeAbilityGroupArgusEntityIds[i] == entityId)
			{
				return true;
			}
		}

		return false;
	}
};
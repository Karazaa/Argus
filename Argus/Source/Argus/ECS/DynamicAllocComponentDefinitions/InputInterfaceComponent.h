// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusMacros.h"
#include "ComponentDefinitions/IdentityComponent.h"
#include "ComponentDependencies/ControlGroup.h"
#include "ComponentDependencies/Timer.h"
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
	TArray<uint16, ArgusContainerAllocator<100u> > m_selectedArgusEntityIds;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<100u> > m_activeAbilityGroupArgusEntityIds;

	ARGUS_IGNORE()
	TArray<ControlGroup, ArgusContainerAllocator<12u> > m_controlGroups;

	ARGUS_IGNORE()
	ESelectedActorsDisplayState m_selectedActorsDisplayState = ESelectedActorsDisplayState::NotChanged;

	ARGUS_IGNORE()
	int8 m_indexOfActiveAbilityGroup = 0;

	ARGUS_IGNORE()
	ETeam m_activePlayerTeam = ETeam::TeamA;

	ARGUS_IGNORE()
	TimerHandle m_doubleClickTimer;

	float m_doubleClickThresholdSeconds = 1.0f;

	float m_doubleClickQueryRange = 1000.0f;

	ARGUS_IGNORE()
	uint16 m_lastSelectedEntityId = ArgusECSConstants::k_maxEntities;

	uint8 m_numControlGroups = 12u;

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

	bool IsEntityIdSelected(uint16 entityId) const
	{
		for (int32 i = 0; i < m_selectedArgusEntityIds.Num(); ++i)
		{
			if (m_selectedArgusEntityIds[i] == entityId)
			{
				return true;
			}
		}

		return false;
	}

	uint16 GetOwningEntityId() const { return ArgusECSConstants::k_singletonEntityId; }
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusECSConstants.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/Timer.h"

UENUM()
enum class EDecalType : uint8 
{
	MoveToLocation,
	AttackMoveToLocation
};

struct ArgusDecalComponent
{
	ARGUS_COMPONENT_SHARED;

	float m_lifetimeSeconds = 1.0f;

	ARGUS_COMP_NO_DATA
	uint16 m_referencingEntityCount = 0u;

	ARGUS_COMP_NO_DATA
	uint16 m_connectedEntityId = ArgusECSConstants::k_maxEntities;

	ARGUS_COMP_NO_DATA
	TimerHandle m_lifetimeTimer;

	ARGUS_COMP_NO_DATA
	EDecalType m_decalType = EDecalType::MoveToLocation;
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ComponentDependencies/CombatInfo.h"
#include "ComponentDependencies/Timer.h"

struct CombatComponent
{
	ARGUS_COMPONENT_SHARED

	uint32 m_baseDamagePerIntervalOrPerSecond = 100u;

	float m_intervalDurationSeconds = 1.0f;

	EAttackType m_attackType = EAttackType::Melee;

	ARGUS_COMP_PROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_attackType == EAttackType::Ranged", EditConditionHides));
	ERangedAttackCapability m_rangedAttackCapability = ERangedAttackCapability::GroundedAndFlying;

	ARGUS_COMP_NO_DATA
	TimerHandle m_attackTimerHandle;

	bool m_isConsideredCombatant = false;
};
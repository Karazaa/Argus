// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"
#include "ArgusMacros.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Melee,
	Ranged
};

struct CombatComponent
{
	ARGUS_COMPONENT_SHARED

	uint32 m_baseDamagePerIntervalOrPerSecond = 100u;

	float m_intervalDurationSeconds = 1.0f;

	EAttackType m_attackType = EAttackType::Melee;

	ARGUS_IGNORE()
	TimerHandle m_attackTimerHandle;
};
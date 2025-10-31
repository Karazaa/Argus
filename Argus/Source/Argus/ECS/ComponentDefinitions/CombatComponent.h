// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ComponentDependencies/Timer.h"
#include "CoreMinimal.h"
#include "ArgusMacros.h"
#include "CombatComponent.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Melee,
	Ranged
};

UENUM(BlueprintType)
enum class ERangedAttackCapability : uint8
{
	GroundedOnly,
	FlyingOnly,
	GroundedAndFlying
};

struct CombatComponent
{
	ARGUS_COMPONENT_SHARED;

	uint32 m_baseDamagePerIntervalOrPerSecond = 100u;

	float m_intervalDurationSeconds = 1.0f;

	EAttackType m_attackType = EAttackType::Melee;

	ARGUS_PROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "m_attackType == EAttackType::Ranged", EditConditionHides));
	ERangedAttackCapability m_rangedAttackCapability = ERangedAttackCapability::GroundedAndFlying;

	ARGUS_IGNORE()
	TimerHandle m_attackTimerHandle;
};
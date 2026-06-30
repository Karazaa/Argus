// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "CombatInfo.generated.h"

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
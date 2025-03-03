// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

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
	uint32 m_baseDamagePerSecond = 100u;

	EAttackType m_attackType = EAttackType::Melee;

	void GetDebugString(FString& debugStringToAppendTo) const
	{
	}
};
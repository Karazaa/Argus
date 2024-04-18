// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EFaction : uint8
{
	None = 0,
	FactionA = 1 << 0,
	FactionB = 1 << 1,
	FactionC = 1 << 2,
	FactionD = 1 << 3,
	FactionE = 1 << 4,
	FactionF = 1 << 5,
	FactionG = 1 << 7
};
ENUM_CLASS_FLAGS(EFaction);

struct IdentityComponent
{
	EFaction m_faction;
	// UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = EFaction))
	uint8 m_allies;
	// UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = EFaction))
	uint8 m_enemies;
};
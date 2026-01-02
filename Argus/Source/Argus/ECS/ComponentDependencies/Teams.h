// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETeam : uint8
{
	None = 0,
	TeamA = 1u << 0,
	TeamB = 1u << 1,
	TeamC = 1u << 2,
	TeamD = 1u << 3,
	TeamE = 1u << 4,
	TeamF = 1u << 5,
	TeamG = 1u << 6,
	TeamH = 1u << 7
};
ENUM_CLASS_FLAGS(ETeam);
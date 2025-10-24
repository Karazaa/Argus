// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EAbilityTypes : uint8
{
	Spawn,
	Attack,
	Heal,
	Construct,
	Vacate,
	TakeOff,
	Land
};
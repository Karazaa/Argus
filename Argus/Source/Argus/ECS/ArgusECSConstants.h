// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

namespace ArgusECSConstants
{
	static constexpr uint16	k_maxEntities = 15000u;
}

// Unreal complains like a dumbass if UENUM is in a namespace. Big dumb.
UENUM()
enum class UEntityPriority : uint16
{
	HighestPriority = 0,
	HighPriority = 1000u,
	MediumPriority = 3000u,
	LowPriority = 6000u,
	LowestPriority = 10000u,
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

#include "FlightCapability.generated.h"

UENUM()
enum class EFlightCapability : uint8
{
	OnlyGrounded,
	OnlyFlying,
	BothGroundedAndFlying
};
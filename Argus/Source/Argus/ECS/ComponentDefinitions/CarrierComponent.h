// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct CarrierComponent
{
	ARGUS_COMPONENT_SHARED

	uint8 m_carrierCapacity = 0u;

	ARGUS_IGNORE()
	TArray<uint16> m_passengerEntityIds;
};
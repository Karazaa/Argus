// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"
#include "CoreMinimal.h"

struct CarrierComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_IGNORE()
	TArray<uint16, ArgusContainerAllocator<4u> > m_passengerEntityIds;

	uint8 m_carrierCapacity = 0u;
};
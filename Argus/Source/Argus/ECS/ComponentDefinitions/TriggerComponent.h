// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"
#include "ComponentDependencies/FlightCapability.h"

struct TriggerComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	TArray<uint16, ArgusContainerAllocator<8u>> m_overlappingEntityIds;

	EFlightCapability m_triggerPlanarOverlaps = EFlightCapability::OnlyGrounded;
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
#include "ArgusSet.h"
#include "ArgusSetAllocator.h"
#include "ComponentDependencies/FlightCapability.h"

struct TriggerComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ArgusSet<uint16, ArgusSetAllocator<8u> > m_overlappingEntityIds;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	ArgusSet<uint16, ArgusSetAllocator<8u> > m_removalStagedEntityIds;

	EFlightCapability m_triggerPlanarOverlaps = EFlightCapability::OnlyGrounded;
};

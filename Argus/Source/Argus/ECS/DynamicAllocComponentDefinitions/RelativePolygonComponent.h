// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "ArgusMacros.h"

struct RelativePolygonComponent
{
	ARGUS_DYNAMIC_COMPONENT_SHARED;

	TArray<FVector2D, ArgusContainerAllocator<0u> > m_relativeUnrealVerticies;

	ARGUS_COMP_NO_DATA ARGUS_COMP_TRANSIENT
	TArray<FVector, ArgusContainerAllocator<0u> > m_worldSpaceUnrealVerticies;
};

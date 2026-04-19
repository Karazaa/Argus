// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ArgusContainerAllocator.h"
#include "ArgusECSConstants.h"
#include "ArgusMacros.h"

UENUM()
enum class EAvoidancePriority : uint8
{
	Lowest,
	Low,
	Medium,
	High,
	Highest
};

struct AvoidanceGroupingComponent
{
	ARGUS_COMPONENT_SHARED;

	ARGUS_NO_DATA
	FVector m_groupAverageLocation = FVector::ZeroVector;

	ARGUS_NO_DATA
	TArray<uint16, ArgusContainerAllocator<10u> > m_entityIdsInGroup;

	ARGUS_NO_DATA
	uint16 m_groupId = ArgusECSConstants::k_maxEntities;

	ARGUS_NO_DATA
	uint16 m_numberOfIdleEntities = 0u;

	EAvoidancePriority m_avoidancePriority = EAvoidancePriority::Lowest;
};
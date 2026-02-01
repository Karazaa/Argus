// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies/ResourceSet.h"

UENUM()
enum class ETeamCommanderDirective : uint8
{
	ConstructResourceSink,
	ExtractResources,
	Scout,

	Count
};

struct TeamCommanderPriority
{
	ETeamCommanderDirective m_directive = ETeamCommanderDirective::Scout;
	EResourceType m_resourceType = EResourceType::Count;
	float m_weight = 0.0f;
};

FORCEINLINE static bool operator<(const TeamCommanderPriority& A, const TeamCommanderPriority& B)
{
	if (A.m_weight < B.m_weight)
	{
		return false;
	}

	if (A.m_weight == B.m_weight)
	{
		return A.m_directive < B.m_directive;
	}

	return true;
}
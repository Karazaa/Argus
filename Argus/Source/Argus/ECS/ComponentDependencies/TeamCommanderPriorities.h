// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies/EntityCategory.h"

class FArchive;

UENUM()
enum class ETeamCommanderDirective : uint8
{
	ConstructResourceSink,
	ExtractResources,
	SpawnUnit,
	Scout,

	Count
};

struct TeamCommanderPriority
{
	ETeamCommanderDirective m_directive = ETeamCommanderDirective::Scout;
	EntityCategory m_entityCategory;
	float m_weight = 0.0f;
};

struct ResourceSourceExtractionData
{
	uint16 m_resourceSourceEntityId = ArgusECSConstants::k_maxEntities;
	uint16 m_resourceSinkEntityId = ArgusECSConstants::k_maxEntities;
	uint16 m_resourceExtractorEntityId = ArgusECSConstants::k_maxEntities;
	uint16 m_resourceSinkConstructorEntityId = ArgusECSConstants::k_maxEntities;

	bool HasSourceEntityId(uint16 entityId) const { return m_resourceSourceEntityId == entityId; }
	void SetSourceEntityId(uint16 entityId) { m_resourceSourceEntityId = entityId; }
};

struct ConstructionData
{
	uint16 m_beingConstructedEntityId = ArgusECSConstants::k_maxEntities;
	uint16 m_constructingOtherEntityId = ArgusECSConstants::k_maxEntities;
};
FArchive& operator<<(FArchive& archive, ResourceSourceExtractionData& controlGroup);


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
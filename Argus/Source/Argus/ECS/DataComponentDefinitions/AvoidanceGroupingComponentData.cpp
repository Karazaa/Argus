// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "AvoidanceGroupingComponentData.h"
#include "ArgusStaticData.h"

void UAvoidanceGroupingComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	AvoidanceGroupingComponent* AvoidanceGroupingComponentRef = entity.AddComponent<AvoidanceGroupingComponent>();
	AvoidanceGroupingComponentRef->m_adjacentEntities = m_adjacentEntities;
	AvoidanceGroupingComponentRef->m_groupAverageLocation = m_groupAverageLocation;
	AvoidanceGroupingComponentRef->m_groupId = m_groupId;
	AvoidanceGroupingComponentRef->m_numberOfIdleEntities = m_numberOfIdleEntities;
}

bool UAvoidanceGroupingComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UAvoidanceGroupingComponentData* otherComponentData = Cast<UAvoidanceGroupingComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

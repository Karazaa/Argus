// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "NavigationComponentData.h"
#include "ArgusStaticData.h"

void UNavigationComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	NavigationComponent* NavigationComponentRef = entity.AddComponent<NavigationComponent>();
	NavigationComponentRef->m_navigationPoints = m_navigationPoints;
	NavigationComponentRef->m_queuedWaypoints = m_queuedWaypoints;
	NavigationComponentRef->m_endedNavigationLocation = m_endedNavigationLocation;
	NavigationComponentRef->m_lastPointIndex = m_lastPointIndex;
}

bool UNavigationComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UNavigationComponentData* otherComponentData = Cast<UNavigationComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

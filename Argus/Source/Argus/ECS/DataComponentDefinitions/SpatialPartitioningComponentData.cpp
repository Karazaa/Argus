// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "SpatialPartitioningComponentData.h"
#include "ArgusStaticData.h"

void USpatialPartitioningComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	SpatialPartitioningComponent* SpatialPartitioningComponentRef = entity.AddComponent<SpatialPartitioningComponent>();
	SpatialPartitioningComponentRef->m_argusKDTree = m_argusKDTree;
}

bool USpatialPartitioningComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const USpatialPartitioningComponentData* otherComponentData = Cast<USpatialPartitioningComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}

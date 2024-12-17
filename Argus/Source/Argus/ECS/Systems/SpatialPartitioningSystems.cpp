// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SpatialPartitioningSystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void SpatialPartitioningSystems::RunSystems(const ArgusEntity& spatialPartitioningEntity)
{
	if (!spatialPartitioningEntity)
	{
		return;
	}

	SpatialPartitioningComponent* spatialPartitioningComponent = spatialPartitioningEntity.GetComponent<SpatialPartitioningComponent>();
	if (!spatialPartitioningComponent)
	{
		return;
	}

	spatialPartitioningComponent->m_argusKDTree.ResetKDTreeWithAverageLocation();

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity retrievedEntity = ArgusEntity::RetrieveEntity(i);
		if (!retrievedEntity)
		{
			continue;
		}

		const TransformComponent* transformComponent = retrievedEntity.GetComponent<TransformComponent>();
		if (!transformComponent)
		{
			continue;
		}

		SpatialPartitioningSystemsComponentArgs components;
		components.m_entity = retrievedEntity;
		components.m_transformComponent = transformComponent;
		components.m_singletonSpatialParitioningComponent = spatialPartitioningComponent;

		spatialPartitioningComponent->m_argusKDTree.InsertArgusEntityIntoKDTree(retrievedEntity);
	}
}

bool SpatialPartitioningSystems::SpatialPartitioningSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (!m_entity || !m_singletonSpatialParitioningComponent)
	{
		ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(SpatialPartitioningSystemsComponentArgs));
		return false;
	}
	return true;
}
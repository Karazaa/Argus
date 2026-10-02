// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "SystemArgumentDefinitions/TriggerSystemsArgs.h"

class TriggerSystems
{
public:
	static void RunSystems(float deltaTime);

private:
	static void UpdateCircleTriggerOverlaps(const TriggerSystemsArgs& components, SpatialPartitioningComponent* spatialPartitioningComponent);
	static void UpdatePolygonTriggerOverlaps(const TriggerSystemsArgs& components, SpatialPartitioningComponent* spatialPartitioningComponent);
	static void RegisterOverlappedEntityId(uint16 entityId, const TriggerSystemsArgs& components);
	static void ProcessStagedRemovalEntityIds(const TriggerSystemsArgs& components);
	static void EnteredTriggerThisFrame(uint16 entityId, const TriggerSystemsArgs& components);
	static void ExitedTriggerThisFrame(uint16 entityId, const TriggerSystemsArgs& components);

	template <typename QueryArray>
	static void UpdateOverlappingEntities(const QueryArray& queryOverlaps, const TriggerSystemsArgs& components)
	{
		for (int32 i = 0; i < queryOverlaps.Num(); ++i)
		{
			RegisterOverlappedEntityId(queryOverlaps[i], components);
		}

		ProcessStagedRemovalEntityIds(components);
	}
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusContainerAllocator.h"
#include "SystemArgumentDefinitions/TriggerSystemsArgs.h"

class TriggerSystems
{
public:
	static void RunSystems(float deltaTime);

private:
	static void UpdateCircleTriggerOverlaps(const TriggerSystemsArgs& components);
	static void UpdatePolygonTriggerOverlaps(const TriggerSystemsArgs& components);
	static void RegisterOverlappedEntityId(uint16 entityId, const TriggerSystemsArgs& components);
	static void ProcessStagedRemovalEntityIds(const TriggerSystemsArgs& components);

	template <typename QueryArray, typename SystemsArgs>
	static void UpdateOverlappingEntities(const QueryArray& queryOverlaps, const TriggerSystemsArgs& components)
	{
		for (int32 i = 0; i < queryOverlaps.Num(); ++i)
		{
			RegisterOverlappedEntityId(queryOverlaps[i], components);
		}

		ProcessStagedRemovalEntityIds(components);
	}
};

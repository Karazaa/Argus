// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderComponentCollection.h"
#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems_AssignEntities
{
public:
	static void RunSystems();

private:
	static void ActUponUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void AssignIdleEntityToWork(ArgusEntity idleEntity, const TeamCommanderComponentCollection& components);
	static bool AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static bool AssignEntityToStartConstructionIfAble(ArgusEntity entity, const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static bool AssignEntityToStartConstructionOfResourceSinkIfAble(ArgusEntity entity, const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static bool AssignEntityToContinueConstructionIfAble(ArgusEntity entity, const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static bool AssignEntityToResourceExtractionIfAble(ArgusEntity entity, const TeamCommanderComponentCollection& components);
	static bool AssignEntityToSpawnUnitIfAble(ArgusEntity entity, const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static bool AssignEntityToScoutingIfAble(ArgusEntity entity, const TeamCommanderComponentCollection& components);

	static bool FindTargetLocForConstructResourceSink(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, const TeamCommanderComponentCollection& components, EResourceType type);
	static ArgusEntity GetNearestSeenResourceSourceToEntity(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, const TeamCommanderComponentCollection& components, EResourceType type, int32& outPairIndex, int32& outExtractionDataIndex);
};

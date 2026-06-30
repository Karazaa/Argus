// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems_AssignEntities
{
public:
	static void RunSystems();

private:
	static void ActUponUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent);
	static bool AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToStartConstructionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToStartConstructionOfResourceSinkIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToContinueConstructionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent);
	static bool AssignEntityToSpawnUnitIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static bool AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);

	static bool FindTargetLocForConstructResourceSink(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, EResourceType type);
	static ArgusEntity GetNearestSeenResourceSourceToEntity(ArgusEntity entity, const TArray<TPair<const UAbilityRecord*, EAbilityIndex>>& abilityIndexPairs, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, EResourceType type, int32& outPairIndex, int32& outExtractionDataIndex);
};

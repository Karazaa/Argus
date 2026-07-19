// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderComponentCollection.h"
#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems_UpdatePriorities
{
public:
	static void RunSystems();

private:
	static void UpdateTeamCommanderPriorities(ArgusEntity teamCommmanderEntity);
	static void UpdateTeamCommanderPriorityCost(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static void UpdateConstructResourceSinkTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static void UpdateResourceExtractionTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static void UpdateSpawnUnitTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static void UpdateSpawnUnitResourceExtractorTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static void UpdateSpawnUnitCombatantTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static void UpdateScoutingTeamPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
	static void UpdateContinueConstructionPriority(const TeamCommanderComponentCollection& components, TeamCommanderPriority& priority);
};

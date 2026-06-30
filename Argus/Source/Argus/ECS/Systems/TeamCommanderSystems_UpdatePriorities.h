// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems_UpdatePriorities
{
public:
	static void RunSystems();

private:
	static void UpdateTeamCommanderPriorities(ArgusEntity teamCommmanderEntity);
	static void UpdateConstructResourceSinkTeamPriority(TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority);
	static void UpdateResourceExtractionTeamPriority(TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority);
	static void UpdateSpawnUnitTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderResourceDataComponent* teamCommanderResourceDataComponent, TeamCommanderPriority& priority);
	static void UpdateScoutingTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateContinueConstructionPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
};

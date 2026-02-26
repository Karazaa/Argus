// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems_UpdatePriorities
{
public:
	static void RunSystems(float deltaTime);

private:
	static void UpdateTeamCommanderPriorities(ArgusEntity teamCommmanderEntity);
	static void UpdateConstructResourceSinkTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateResourceExtractionTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateSpawnUnitTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
	static void UpdateScoutingTeamPriority(TeamCommanderComponent* teamCommanderComponent, TeamCommanderPriority& priority);
};

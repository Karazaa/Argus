// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems
{
public:
	static void RunSystems(float deltaTime);

private:
	static void UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components);
	static void UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateTeamCommanderPerEntityOnOtherTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);

	static void ActUponUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent);
	static void AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);
	static void ClearUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
};

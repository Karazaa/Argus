// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems
{
public:
	static void RunSystems(float deltaTime);

private:
	static void UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components);
	static void ClearUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
};

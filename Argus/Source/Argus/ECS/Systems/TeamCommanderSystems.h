// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/TeamCommanderSystemsArgs.h"

class TeamCommanderSystems
{
public:
	static void RunSystems(float deltaTime);
	static void InitializeRevealedAreas(TeamCommanderComponent* teamCommanderComponent);

private:
	static void ClearUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);

	static void UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components);
	static void UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
	static void UpdateTeamCommanderPerNeutralEntity(const TeamCommanderSystemsArgs& components, ArgusEntity teamCommanderEntity);
	static void UpdateRevealedAreasPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);

	static void UpdateTeamCommanderPriorities(ArgusEntity teamCommmanderEntity);

	static void ActUponUpdatesPerCommanderEntity(ArgusEntity teamCommmanderEntity);
	static void AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent);
	static bool AssignIdleEntityToDirectiveIfAble(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent, ETeamCommanderDirective directive);
	static bool AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);
	static bool AssignEntityToScoutingIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent);

	static int32 GetRevealedAreaIndexFromWorldSpaceLocation(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent);
};

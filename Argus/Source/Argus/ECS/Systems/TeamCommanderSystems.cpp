// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void TeamCommanderSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TeamCommanderSystems::RunSystems);

	ArgusEntity::IterateTeamEntities(&TeamCommanderSystems::ClearUpdatesPerCommanderEntity);
	ArgusEntity::IterateSystemsArgs<TeamCommanderSystemsArgs>(TeamCommanderSystems::UpdateTeamCommanderPerEntity);
}

void TeamCommanderSystems::UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_entity.IsAlive())
	{
		return;
	}

	TeamCommanderComponent* teamCommanderComponent = ArgusEntity::GetTeamEntity(components.m_identityComponent->m_team).GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	teamCommanderComponent->m_numLivingUnits++;
	if (components.m_entity.IsIdle())
	{
		teamCommanderComponent->m_idleEntityIdsForTeam.Add(components.m_entity.GetId());
	}

	if (components.m_taskComponent->m_resourceExtractionState != EResourceExtractionState::None)
	{
		teamCommanderComponent->m_numResourceExtractors++;
	}
}

void TeamCommanderSystems::ClearUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	teamCommanderComponent->m_idleEntityIdsForTeam.Reset();
	teamCommanderComponent->m_numResourceExtractors = 0u;
	teamCommanderComponent->m_numLivingUnits = 0u;
}

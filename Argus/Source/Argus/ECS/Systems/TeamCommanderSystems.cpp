// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamCommanderSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/ResourceSystems.h"

void TeamCommanderSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TeamCommanderSystems::RunSystems);

	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);
	ArgusEntity::IterateSystemsArgs<TeamCommanderSystemsArgs>(TeamCommanderSystems::UpdateTeamCommanderPerEntity);
	ArgusEntity::IterateTeamEntities(TeamCommanderSystems::ActUponUpdatesPerCommanderEntity);
}

void TeamCommanderSystems::UpdateTeamCommanderPerEntity(const TeamCommanderSystemsArgs& components)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_entity.IsAlive())
	{
		return;
	}

	TeamCommanderComponent* teamCommanderComponent = ArgusEntity::GetTeamEntity(components.m_identityComponent->m_team).GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	if (components.m_entity.IsOnTeam(teamCommanderComponent->m_teamToCommand))
	{
		UpdateTeamCommanderPerEntityOnTeam(components, teamCommanderComponent);
	}
	else
	{
		UpdateTeamCommanderPerEntityOnOtherTeam(components, teamCommanderComponent);
	}
}

void TeamCommanderSystems::UpdateTeamCommanderPerEntityOnTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntityOnTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

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

void TeamCommanderSystems::UpdateTeamCommanderPerEntityOnOtherTeam(const TeamCommanderSystemsArgs& components, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::UpdateTeamCommanderPerEntityOnOtherTeam);

	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (ResourceComponent* resourceComponent = components.m_entity.GetComponent<ResourceComponent>())
	{
		if (resourceComponent->m_resourceComponentOwnerType == EResourceComponentOwnerType::Source && components.m_identityComponent->WasEverSeenBy(teamCommanderComponent->m_teamToCommand))
		{
			teamCommanderComponent->m_seenResourceSourceEntityIds.Add(components.m_entity.GetId());
		}
	}
}

void TeamCommanderSystems::ActUponUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ActUponUpdatesPerCommanderEntity);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (int32 i = 0; i < teamCommanderComponent->m_idleEntityIdsForTeam.Num(); ++i)
	{
		AssignIdleEntityToWork(ArgusEntity::RetrieveEntity(teamCommanderComponent->m_idleEntityIdsForTeam[i]), teamCommanderComponent);
	}
}

void TeamCommanderSystems::AssignIdleEntityToWork(ArgusEntity idleEntity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignIdleEntityToWork);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	AssignEntityToResourceExtractionIfAble(idleEntity, teamCommanderComponent);
}

void TeamCommanderSystems::AssignEntityToResourceExtractionIfAble(ArgusEntity entity, TeamCommanderComponent* teamCommanderComponent)
{
	ARGUS_TRACE(TeamCommanderSystems::AssignEntityToResourceExtractionIfAble);
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	for (int32 i = 0; i < teamCommanderComponent->m_seenResourceSourceEntityIds.Num(); ++i)
	{
		ArgusEntity resourceSourceEntity = ArgusEntity::RetrieveEntity(teamCommanderComponent->m_seenResourceSourceEntityIds[i]);
		if (!ResourceSystems::CanEntityExtractResourcesFromOtherEntity(entity, resourceSourceEntity))
		{
			continue;
		}

		// TODO JAMES: Do assignment then break.
	}
}

void TeamCommanderSystems::ClearUpdatesPerCommanderEntity(ArgusEntity teamEntity)
{
	ARGUS_TRACE(TeamCommanderSystems::ClearUpdatesPerCommanderEntity);

	TeamCommanderComponent* teamCommanderComponent = teamEntity.GetComponent<TeamCommanderComponent>();
	ARGUS_RETURN_ON_NULL(teamCommanderComponent, ArgusECSLog);

	teamCommanderComponent->m_idleEntityIdsForTeam.Reset();
	teamCommanderComponent->m_seenResourceSourceEntityIds.Reset();
	teamCommanderComponent->m_numResourceExtractors = 0u;
	teamCommanderComponent->m_numLivingUnits = 0u;
}

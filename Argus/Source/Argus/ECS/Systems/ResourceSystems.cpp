// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

bool ResourceSystems::CanEntityAffordResourceChange(const ArgusEntity& entity, const FResourceSet& resourceChange)
{
	ResourceComponent* teamResourceComponent = GetTeamResourceComponentForEntity(entity);
	if (!teamResourceComponent)
	{
		return false;
	}

	return teamResourceComponent->m_currentResources.CanAffordResourceChange(resourceChange);
}

bool ResourceSystems::ApplyResourceChangeIfAffordable(const ArgusEntity& entity, const FResourceSet& resourceChange)
{
	ResourceComponent* teamResourceComponent = GetTeamResourceComponentForEntity(entity);
	if (!teamResourceComponent)
	{
		return false;
	}

	if (!teamResourceComponent->m_currentResources.CanAffordResourceChange(resourceChange))
	{
		return false;
	}

	teamResourceComponent->m_currentResources.ApplyResourceChange(resourceChange);
	return true;
}

ResourceComponent* ResourceSystems::GetTeamResourceComponentForEntity(const ArgusEntity& entity)
{
	if (!entity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Passed in invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity));
		return nullptr;
	}

	IdentityComponent* identityComponent = entity.GetComponent<IdentityComponent>();
	if (!identityComponent)
	{
		return nullptr;
	}

	ArgusEntity teamEntity = ArgusEntity::GetTeamEntity(identityComponent->m_team);
	if (!teamEntity)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Could not retrieve a valid %s from %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(ArgusEntity::GetTeamEntity));
		return nullptr;
	}

	return teamEntity.GetComponent<ResourceComponent>();
}

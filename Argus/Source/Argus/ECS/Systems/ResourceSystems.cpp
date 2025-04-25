// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/TargetingSystems.h"

void ResourceSystems::RunSystems(float deltaTime)
{
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ResourceComponents components;
		components.m_entity = ArgusEntity::RetrieveEntity(i);
		if (!components.m_entity)
		{
			continue;
		}

		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_resourceComponent = components.m_entity.GetComponent<ResourceComponent>();
		components.m_resourceExtractionComponent = components.m_entity.GetComponent<ResourceExtractionComponent>();
		components.m_targetingComponent = components.m_entity.GetComponent<TargetingComponent>();
		if (!components.m_taskComponent || !components.m_resourceComponent ||
			!components.m_resourceExtractionComponent || !components.m_targetingComponent)
		{
			continue;
		}

		ProcessResourceExtraction(components);
	}
};

bool ResourceSystems::ResourceComponents::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (!m_entity || !m_taskComponent || !m_resourceComponent || !m_resourceExtractionComponent || !m_targetingComponent)
	{
		ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(ResourceComponents));
		return false;
	}

	return true;
}

void ResourceSystems::ProcessResourceExtraction(const ResourceComponents& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_taskComponent->m_resourceExtractionState)
	{
		case EResourceExtractionState::None:
			break;
		case EResourceExtractionState::Extracting:
		{
			if (!components.m_targetingComponent->HasEntityTarget())
			{
				components.m_taskComponent->m_resourceExtractionState = EResourceExtractionState::None;
				return;
			}

			const ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
			if (TargetingSystems::IsInMeleeRangeOfOtherEntity(components.m_entity, targetEntity) && CanEntityExtractResourcesFromOtherEntity(components.m_entity, targetEntity))
			{
				// TODO JAMES: Actually do resource extraction.
			}
			break;
		}
		case EResourceExtractionState::Depositing:
			break;
	}
}

bool ResourceSystems::CanEntityExtractResourcesFromOtherEntity(const ArgusEntity& entity, const ArgusEntity& otherEntity)
{
	if (!entity || !otherEntity)
	{
		return false;
	}

	const ResourceExtractionComponent* resourceExtractionComponent = entity.GetComponent<ResourceExtractionComponent>();
	if (!resourceExtractionComponent)
	{
		return false;
	}

	const ResourceComponent* otherEntityResourceComponent = otherEntity.GetComponent<ResourceComponent>();
	if (!otherEntityResourceComponent)
	{
		return false;
	}

	return otherEntityResourceComponent->m_currentResources.CanAffordResourceChange(resourceExtractionComponent->m_resourcesToExtract);
}

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

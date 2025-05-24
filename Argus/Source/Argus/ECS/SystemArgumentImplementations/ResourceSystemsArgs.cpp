// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SystemArgumentDefinitions/ResourceSystemsArgs.h"
#include "ArgusLogging.h"

bool ResourceSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (!entity)
	{
		return false;
	}

	m_entity = entity;
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_resourceComponent = entity.GetComponent<ResourceComponent>();
	m_resourceExtractionComponent = entity.GetComponent<ResourceExtractionComponent>();
	m_targetingComponent = entity.GetComponent<TargetingComponent>();

	if (!m_taskComponent || !m_resourceComponent || !m_resourceExtractionComponent || !m_targetingComponent)
	{
		return false;
	}

	return true;
}

bool ResourceSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_resourceComponent && m_resourceExtractionComponent && m_targetingComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(ResourceSystemsArgs));

	return false;
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SystemArgumentDefinitions/ConstructionSystemsArgs.h"
#include "ArgusLogging.h"

bool ConstructionSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (!entity)
	{
		return false;
	}

	m_entity = entity;
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_constructionComponent = entity.GetComponent<ConstructionComponent>();

	if (!m_taskComponent || !m_constructionComponent)
	{
		return false;
	}

	return true;
}

bool ConstructionSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_constructionComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(ConstructionSystemsArgs));

	return false;
}
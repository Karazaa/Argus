// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "SystemArgumentDefinitions\ConstructionSystemsArgs.h"
#include "ArgusLogging.h"

bool ConstructionSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (UNLIKELY(!entity))
	{
		return false;
	}

	m_entity = entity;
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_constructionComponent = entity.GetComponent<ConstructionComponent>();

	if (!m_entity || !m_taskComponent)
	{
		return false;
	}

	return true;
}

bool ConstructionSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (UNLIKELY(!m_entity || !m_taskComponent))
	{
		ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(ConstructionSystemsArgs));
		return false;
	}

	return true;
}

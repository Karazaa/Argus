// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SystemArgumentDefinitions/TargetingSystemsArgs.h"
#include "ArgusLogging.h"

bool TargetingSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (!entity)
	{
		return false;
	}

	m_entity = entity;
	m_targetingComponent = entity.GetComponent<TargetingComponent>();
	m_transformComponent = entity.GetComponent<TransformComponent>();

	if (!m_targetingComponent || !m_transformComponent)
	{
		return false;
	}

	return true;
}

bool TargetingSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_targetingComponent && m_transformComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(TargetingSystemsArgs));

	return true;
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SystemArgumentDefinitions/NavigationSystemsArgs.h"
#include "ArgusLogging.h"

bool NavigationSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (!entity)
	{
		return false;
	}

	m_entity = entity;
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_navigationComponent = entity.GetComponent<NavigationComponent>();
	m_targetingComponent = entity.GetComponent<TargetingComponent>();
	m_transformComponent = entity.GetComponent<TransformComponent>();
	m_velocityComponent = entity.GetComponent<VelocityComponent>();

	if (!m_taskComponent || !m_navigationComponent || !m_transformComponent || !m_targetingComponent || !m_velocityComponent)
	{
		return false;
	}

	return true;
}

bool NavigationSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_navigationComponent && m_transformComponent && m_targetingComponent && m_velocityComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(NavigationSystemsArgs));

	return false;
}
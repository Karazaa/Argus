// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "SystemArgumentDefinitions\TransformSystemsArgs.h"
#include "ArgusLogging.h"

bool TransformSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (UNLIKELY(!entity))
	{
		return false;
	}

	m_entity = entity;
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_transformComponent = entity.GetComponent<TransformComponent>();
	m_velocityComponent = entity.GetComponent<VelocityComponent>();
	m_navigationComponent = entity.GetComponent<NavigationComponent>();
	m_targetingComponent = entity.GetComponent<TargetingComponent>();

	if (!m_entity || !m_taskComponent || !m_transformComponent || !m_velocityComponent || !m_navigationComponent || !m_targetingComponent)
	{
		return false;
	}

	return true;
}

bool TransformSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (UNLIKELY(!m_entity || !m_taskComponent || !m_transformComponent || !m_velocityComponent || !m_navigationComponent || !m_targetingComponent))
	{
		ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(TransformSystemsArgs));
		return false;
	}

	return true;
}

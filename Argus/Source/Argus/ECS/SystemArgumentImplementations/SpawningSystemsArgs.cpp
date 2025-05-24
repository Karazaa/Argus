// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SystemArgumentDefinitions/SpawningSystemsArgs.h"
#include "ArgusLogging.h"

bool SpawningSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (!entity)
	{
		return false;
	}

	m_entity = entity;
	m_spawningComponent = entity.GetComponent<SpawningComponent>();
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_targetingComponent = entity.GetComponent<TargetingComponent>();
	m_transformComponent = entity.GetComponent<TransformComponent>();

	if (!m_taskComponent || !m_spawningComponent || !m_targetingComponent || !m_transformComponent)
	{
		return false;
	}

	return true;
}

bool SpawningSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_taskComponent && m_spawningComponent && m_targetingComponent && m_transformComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(SpawningSystemsArgs));

	return false;
}
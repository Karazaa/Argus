// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SystemArgumentDefinitions/CombatSystemsArgs.h"
#include "ArgusLogging.h"

bool CombatSystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (!entity)
	{
		return false;
	}

	m_entity = entity;
	m_combatComponent = entity.GetComponent<CombatComponent>();
	m_identityComponent = entity.GetComponent<IdentityComponent>();
	m_targetingComponent = entity.GetComponent<TargetingComponent>();
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_transformComponent = entity.GetComponent<TransformComponent>();

	if (!m_identityComponent || !m_targetingComponent || !m_taskComponent || !m_transformComponent)
	{
		return false;
	}

	return true;
}

bool CombatSystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_identityComponent && m_targetingComponent && m_taskComponent && m_transformComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(CombatSystemsArgs));

	return false;
}
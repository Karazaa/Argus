// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "SystemArgumentDefinitions/AbilitySystemsArgs.h"
#include "ArgusLogging.h"

bool AbilitySystemsArgs::PopulateArguments(const ArgusEntity& entity)
{
	if (!entity)
	{
		return false;
	}

	m_entity = entity;
	m_taskComponent = entity.GetComponent<TaskComponent>();
	m_abilityComponent = entity.GetComponent<AbilityComponent>();
	
	if (!m_taskComponent || !m_abilityComponent)
	{
		return false;
	}

	return true;
}

bool AbilitySystemsArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_abilityComponent && m_taskComponent && m_reticleComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(AbilitySystemsArgs));

	return false;
}
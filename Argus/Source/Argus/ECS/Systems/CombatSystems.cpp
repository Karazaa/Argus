// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "CombatSystems.h"

void CombatSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(CombatSystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i < ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		CombatSystemsComponentArgs components;
		components.m_entity = ArgusEntity::RetrieveEntity(i);
		if (!components.m_entity)
		{
			continue;
		}

		components.m_combatComponent = components.m_entity.GetComponent<CombatComponent>();
		components.m_identityComponent = components.m_entity.GetComponent<IdentityComponent>();
		components.m_targetingComponent = components.m_entity.GetComponent<TargetingComponent>();
		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_transformComponent = components.m_entity.GetComponent<TransformComponent>();
		if (!components.m_entity || components.m_combatComponent || !components.m_taskComponent || 
			!components.m_transformComponent || !components.m_identityComponent || !components.m_targetingComponent)
		{
			continue;
		}

		ProcessCombatTaskCommands(deltaTime, components);
	}
}

bool CombatSystems::CombatSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (m_entity && m_identityComponent && m_targetingComponent && m_taskComponent && m_transformComponent)
	{
		return true;
	}

	ArgusLogging::LogInvalidComponentReferences(functionName, ARGUS_NAMEOF(CombatSystemsComponentArgs));

	return false;
}

void CombatSystems::ProcessCombatTaskCommands(float deltaTime, const CombatSystemsComponentArgs& components)
{
	ARGUS_TRACE(CombatSystems::ProcessCombatTaskCommands);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
}

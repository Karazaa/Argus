// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ConstructionSystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void ConstructionSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(i);
		if (!potentialEntity)
		{
			continue;
		}

		ConstructionSystemsComponentArgs components;
		components.m_entity = potentialEntity;
		components.m_taskComponent = potentialEntity.GetComponent<TaskComponent>();
		components.m_constructionComponent = potentialEntity.GetComponent<ConstructionComponent>();

		if (!components.m_entity || !components.m_taskComponent || !components.m_constructionComponent)
		{
			continue;
		}

		ProcessConstructionState(components, deltaTime);
	}
}

bool ConstructionSystems::ConstructionSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (!m_taskComponent || !m_constructionComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Construction Systems were run with invalid component arguments passed."), functionName);
		return false;
	}

	return true;
}

void ConstructionSystems::ProcessConstructionState(const ConstructionSystemsComponentArgs& components, float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::ProcessConstructionState);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	switch (components.m_constructionComponent->m_constructionType)
	{
		case EConstructionType::Automatic:
			ProcessAutomaticConstruction(components, deltaTime);
			break;
		case EConstructionType::Manual:
			ProcessManualConstruction(components, deltaTime);
			break;
		default:
			break;
	}
}

void ConstructionSystems::ProcessAutomaticConstruction(const ConstructionSystemsComponentArgs& components, float deltaTime)
{

}

void ConstructionSystems::ProcessManualConstruction(const ConstructionSystemsComponentArgs& components, float deltaTime)
{

}

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

		switch (components.m_taskComponent->m_constructionState)
		{
			case ConstructionState::BeingConstructed:
				ProcessBeingConstructedState(components, deltaTime);
				break;
			case ConstructionState::ConstructingOther:
				break;
			case ConstructionState::ConstructionFinished:
				components.m_taskComponent->m_constructionState = ConstructionState::None;
				break;
			default:
				break;
		}
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

void ConstructionSystems::ProcessBeingConstructedState(const ConstructionSystemsComponentArgs& components, float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::ProcessBeingConstructedState);

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

	if (components.m_constructionComponent->m_currentWorkSeconds >= components.m_constructionComponent->m_requiredWorkSeconds)
	{
		components.m_taskComponent->m_constructionState = ConstructionState::ConstructionFinished;
	}
}

void ConstructionSystems::ProcessAutomaticConstruction(const ConstructionSystemsComponentArgs& components, float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::ProcessAutomaticConstruction);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_constructionComponent->m_automaticConstructionTimerHandle.IsTimerComplete(components.m_entity))
	{
		components.m_constructionComponent->m_automaticConstructionTimerHandle.FinishTimerHandling(components.m_entity);
		components.m_constructionComponent->m_currentWorkSeconds = components.m_constructionComponent->m_requiredWorkSeconds;
		return;
	}

	const float timeElapsedProportion = 1.0f - components.m_constructionComponent->m_automaticConstructionTimerHandle.GetTimeRemainingProportion(components.m_entity);
	components.m_constructionComponent->m_currentWorkSeconds = timeElapsedProportion * components.m_constructionComponent->m_requiredWorkSeconds;
}

void ConstructionSystems::ProcessManualConstruction(const ConstructionSystemsComponentArgs& components, float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::ProcessManualConstruction);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
}

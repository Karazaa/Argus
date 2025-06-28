// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ConstructionSystems.h"
#include "ArgusLogging.h"
#include "Systems/TargetingSystems.h"

void ConstructionSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::RunSystems);

	ConstructionSystemsArgs components;
	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		if (!components.PopulateArguments(ArgusEntity::RetrieveEntity(i)))
		{
			continue;
		}

		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			continue;
		}

		ProcessConstructionTaskCommands(deltaTime, components);
	}
}

bool ConstructionSystems::CanEntityConstructOtherEntity(const ArgusEntity& potentialConstructor, const ArgusEntity& potentialConstructee)
{
	const ConstructionComponent* constructionComponent = potentialConstructee.GetComponent<ConstructionComponent>();
	if (!constructionComponent)
	{
		return false;
	}

	const AbilityComponent* potentialConstructorAbilityComponent = potentialConstructor.GetComponent<AbilityComponent>();
	if (!potentialConstructorAbilityComponent || !potentialConstructorAbilityComponent->HasAbility(constructionComponent->m_constructionAbilityRecordId))
	{
		return false;
	}

	const TaskComponent* taskComponent = potentialConstructee.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		return false;
	}

	const TransformComponent* constructorTransformComponent = potentialConstructor.GetComponent<TransformComponent>();
	const TransformComponent* constructeeTransformComponent = potentialConstructee.GetComponent<TransformComponent>();
	if (!constructeeTransformComponent || !constructeeTransformComponent)
	{
		return false;
	}

	return (taskComponent->m_constructionState == EConstructionState::BeingConstructed) && (constructionComponent->m_constructionType == EConstructionType::Manual);
}

void ConstructionSystems::ProcessConstructionTaskCommands(float deltaTime, const ConstructionSystemsArgs& components)
{
	switch (components.m_taskComponent->m_constructionState)
	{
		case EConstructionState::BeingConstructed:
			if (components.m_constructionComponent)
			{
				ProcessBeingConstructedState(deltaTime, components);
			}
			break;
		case EConstructionState::ConstructingOther:
			ProcessConstructingOtherState(deltaTime, components);
			break;
		case EConstructionState::ConstructionFinished:
			components.m_taskComponent->m_constructionState = EConstructionState::None;
			components.m_constructionComponent->m_currentWorkSeconds = 0.0f;
			break;
		default:
			break;
	}
}

void ConstructionSystems::ProcessBeingConstructedState(float deltaTime, const ConstructionSystemsArgs& components)
{
	ARGUS_TRACE(ConstructionSystems::ProcessBeingConstructedState);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME) || !components.m_constructionComponent)
	{
		return;
	}

	switch (components.m_constructionComponent->m_constructionType)
	{
		case EConstructionType::Automatic:
			ProcessAutomaticConstruction(deltaTime, components);
			break;
		case EConstructionType::Manual:
			break;
		default:
			break;
	}

	if (components.m_constructionComponent->m_currentWorkSeconds >= components.m_constructionComponent->m_requiredWorkSeconds)
	{
		components.m_taskComponent->m_constructionState = EConstructionState::ConstructionFinished;
	}
}

void ConstructionSystems::ProcessConstructingOtherState(float deltaTime, const ConstructionSystemsArgs& components)
{
	ARGUS_TRACE(ConstructionSystems::ProcessConstructingOtherState);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	const TargetingComponent* targetingComponent = components.m_entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(TargetingComponent*));
		components.m_taskComponent->m_constructionState = EConstructionState::None;
		return;
	}

	const ArgusEntity constructee = ArgusEntity::RetrieveEntity(targetingComponent->m_targetEntityId);
	if (!constructee)
	{
		components.m_taskComponent->m_constructionState = EConstructionState::None;
		return;
	}

	if (!CanEntityConstructOtherEntity(components.m_entity, constructee))
	{
		components.m_taskComponent->m_constructionState = EConstructionState::None;
		return;
	}

	if (!TargetingSystems::IsInMeleeRangeOfOtherEntity(components.m_entity, constructee))
	{
		return;
	}

	ConstructionComponent* constructeeConstructionComponent = constructee.GetComponent<ConstructionComponent>();
	if (!constructeeConstructionComponent)
	{
		components.m_taskComponent->m_constructionState = EConstructionState::None;
		return;
	}

	constructeeConstructionComponent->m_currentWorkSeconds += deltaTime;

	if (constructeeConstructionComponent->m_currentWorkSeconds >= constructeeConstructionComponent->m_requiredWorkSeconds)
	{
		constructeeConstructionComponent->m_currentWorkSeconds = constructeeConstructionComponent->m_requiredWorkSeconds;
		components.m_taskComponent->m_constructionState = EConstructionState::None;
	}
}

void ConstructionSystems::ProcessAutomaticConstruction(float deltaTime, const ConstructionSystemsArgs& components)
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

	const float timeElapsedProportion = components.m_constructionComponent->m_automaticConstructionTimerHandle.GetTimeElapsedProportion(components.m_entity);
	components.m_constructionComponent->m_currentWorkSeconds = timeElapsedProportion * components.m_constructionComponent->m_requiredWorkSeconds;
}

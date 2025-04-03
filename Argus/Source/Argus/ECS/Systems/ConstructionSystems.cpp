// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ConstructionSystems.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"

void ConstructionSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::RunSystems);

	for (uint16 i = ArgusEntity::GetLowestTakenEntityId(); i <= ArgusEntity::GetHighestTakenEntityId(); ++i)
	{
		ConstructionSystemsComponentArgs components;
		components.m_entity = ArgusEntity::RetrieveEntity(i);
		if (!components.m_entity)
		{
			continue;
		}

		if (components.m_entity.IsKillable() && !components.m_entity.IsAlive())
		{
			continue;
		}

		components.m_entity = components.m_entity;
		components.m_taskComponent = components.m_entity.GetComponent<TaskComponent>();
		components.m_constructionComponent = components.m_entity.GetComponent<ConstructionComponent>();

		if (!components.m_entity || !components.m_taskComponent)
		{
			continue;
		}

		ProcessConstructionTaskCommands(deltaTime, components);
	}
}

bool ConstructionSystems::CanEntityConstructOtherEntity(const ArgusEntity& potentialConstructor, const ArgusEntity& potentialConstructee)
{
	if (!potentialConstructor)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid reference to %s, %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(potentialConstructor));
		return false;
	}

	if (!potentialConstructee)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Invalid reference to %s, %s."), ARGUS_FUNCNAME, ARGUS_NAMEOF(ArgusEntity), ARGUS_NAMEOF(potentialConstructee));
		return false;
	}

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

	return (taskComponent->m_constructionState == EConstructionState::BeingConstructed) && (constructionComponent->m_constructionType == EConstructionType::Manual);
}

bool ConstructionSystems::ConstructionSystemsComponentArgs::AreComponentsValidCheck(const WIDECHAR* functionName) const
{
	if (!m_entity || !m_taskComponent)
	{
		ARGUS_LOG(ArgusECSLog, Error, TEXT("[%s] Construction Systems were run with invalid component arguments passed."), functionName);
		return false;
	}

	return true;
}

void ConstructionSystems::ProcessConstructionTaskCommands(float deltaTime, const ConstructionSystemsComponentArgs& components)
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

void ConstructionSystems::ProcessBeingConstructedState(float deltaTime, const ConstructionSystemsComponentArgs& components)
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

void ConstructionSystems::ProcessConstructingOtherState(float deltaTime, const ConstructionSystemsComponentArgs& components)
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

	const TransformComponent* constructorTransformComponent = components.m_entity.GetComponent<TransformComponent>();
	const TransformComponent* constructeeTransformComponent = constructee.GetComponent<TransformComponent>();
	if (!constructeeTransformComponent || !constructeeTransformComponent)
	{
		components.m_taskComponent->m_constructionState = EConstructionState::None;
		return;
	}

	if (FVector::DistSquared(constructorTransformComponent->m_location, constructeeTransformComponent->m_location) > 
		FMath::Square(targetingComponent->m_meleeRange))
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

void ConstructionSystems::ProcessAutomaticConstruction(float deltaTime, const ConstructionSystemsComponentArgs& components)
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

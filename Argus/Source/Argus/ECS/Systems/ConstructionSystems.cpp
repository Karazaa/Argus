// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ConstructionSystems.h"
#include "ArgusLogging.h"
#include "Systems/TargetingSystems.h"

void ConstructionSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(ConstructionSystems::RunSystems);

	ArgusEntity::IterateSystemsArgs<ConstructionSystemsArgs>([deltaTime](ConstructionSystemsArgs& components) 
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		ProcessConstructionTaskCommands(deltaTime, components);
	});
}

bool ConstructionSystems::CanEntityConstructOtherEntity(ArgusEntity potentialConstructor, ArgusEntity potentialConstructee)
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
		case EConstructionState::DispatchedToConstructOther:
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

	if (!components.m_targetingComponent)
	{
		components.m_taskComponent->m_constructionState = EConstructionState::None;
		return;
	}

	const ArgusEntity constructee = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
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
		components.m_taskComponent->m_constructionState = EConstructionState::DispatchedToConstructOther;
		return;
	}
	components.m_taskComponent->m_constructionState = EConstructionState::ConstructingOther;

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
		StopConstructingOther(components);
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

void ConstructionSystems::StopConstructingOther(const ConstructionSystemsArgs& components)
{
	components.m_taskComponent->m_constructionState = EConstructionState::None;
	if (components.m_targetingComponent)
	{
		components.m_targetingComponent->m_targetEntityId = ArgusECSConstants::k_maxEntities;
	}
	if (components.m_taskComponent->m_movementState == EMovementState::InRangeOfTargetEntity)
	{
		components.m_taskComponent->m_movementState = EMovementState::None;
	}
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TaskSystems.h"
#include "ArgusEntity.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/TargetingSystems.h"

void TaskSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TaskSystems::RunSystems);

	TaskSystemsArgs components;
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

		ProcessIdleEntity(components);
		ProcessInRangeOfTargetEntity(components);
	}
}

void TaskSystems::ProcessIdleEntity(const TaskSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_entity.IsIdle())
	{
		return;
	}

	for (int32 i = 0; i < components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInMeleeRange().Num(); ++i)
	{
		if (ProcessDispatchingForEntityPair(components, components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInMeleeRange()[i]))
		{
			return;
		}
	}
	for (int32 i = 0; i < components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInRangedRange().Num(); ++i)
	{
		if (ProcessDispatchingForEntityPair(components, components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInRangedRange()[i]))
		{
			return;
		}
	}
	for (int32 i = 0; i < components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInSightRange().Num(); ++i)
	{
		if (ProcessDispatchingForEntityPair(components, components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInSightRange()[i]))
		{
			return;
		}
	}
}

bool TaskSystems::ProcessDispatchingForEntityPair(const TaskSystemsArgs& components, uint16 potentialTargetEntityId)
{
	ArgusEntity potentialTargetEntity = ArgusEntity::RetrieveEntity(potentialTargetEntityId);

	if (DispatchToConstructionIfAble(components, potentialTargetEntity))
	{
		return true;
	}
	if (DispatchToCombatIfAble(components, potentialTargetEntity))
	{
		return true;
	}

	return false;
}

bool TaskSystems::DispatchToConstructionIfAble(const TaskSystemsArgs& components, const ArgusEntity& potentialTargetEntity)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (!ConstructionSystems::CanEntityConstructOtherEntity(components.m_entity, potentialTargetEntity))
	{
		return false;
	}

	components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	components.m_taskComponent->m_constructionState = EConstructionState::DispatchedToConstructOther;
	components.m_targetingComponent->m_targetEntityId = potentialTargetEntity.GetId();

	return true;
}

bool TaskSystems::DispatchToCombatIfAble(const TaskSystemsArgs& components, const ArgusEntity& potentialTargetEntity)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return false;
	}

	if (!CombatSystems::CanEntityAttackOtherEntity(components.m_entity, potentialTargetEntity))
	{
		return false;
	}

	components.m_taskComponent->m_movementState = EMovementState::ProcessMoveToEntityCommand;
	components.m_taskComponent->m_combatState = ECombatState::DispatchedToAttack;
	components.m_targetingComponent->m_targetEntityId = potentialTargetEntity.GetId();

	return true;
}

void TaskSystems::ProcessInRangeOfTargetEntity(const TaskSystemsArgs& components)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_movementState != EMovementState::InRangeOfTargetEntity)
	{
		return;
	}

	ArgusEntity targetEntity = ArgusEntity::RetrieveEntity(components.m_targetingComponent->m_targetEntityId);
	if (components.m_entity.IsInRangeOfOtherEntity(targetEntity, TargetingSystems::GetRangeToUseForOtherEntity(components.m_entity, targetEntity)))
	{
		return;
	}

	components.m_taskComponent->m_movementState = EMovementState::MoveToEntity;
}

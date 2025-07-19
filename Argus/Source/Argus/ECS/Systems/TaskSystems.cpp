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

	const TArray<uint16>& meleeRangeEntities = components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInMeleeRange();
	const TArray<uint16>& rangedRangeEntities = components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInRangedRange();
	const TArray<uint16>& sightRangeEntities = components.m_nearbyEntitiesComponent->m_nearbyEntities.GetEntityIdsInSightRange();

	for (int32 i = 0; i < meleeRangeEntities.Num(); ++i)
	{
		if (ProcessDispatchingForEntityPair(components, meleeRangeEntities[i]))
		{
			return;
		}
	}
	for (int32 i = 0; i < rangedRangeEntities.Num(); ++i)
	{
		if (ProcessDispatchingForEntityPair(components, rangedRangeEntities[i]))
		{
			return;
		}
	}
	for (int32 i = 0; i < sightRangeEntities.Num(); ++i)
	{
		if (ProcessDispatchingForEntityPair(components, sightRangeEntities[i]))
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
	components.m_taskComponent->m_constructionState = EConstructionState::ConstructingOther;
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
	components.m_taskComponent->m_combatState = ECombatState::ShouldAttack;
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

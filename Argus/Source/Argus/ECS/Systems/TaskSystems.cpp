// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TaskSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/CombatSystems.h"
#include "Systems/ConstructionSystems.h"
#include "Systems/NavigationSystems.h"
#include "Systems/TargetingSystems.h"

void TaskSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(TaskSystems::RunSystems);

	ArgusIterators::IterateSystemsArgs<TaskSystemsArgs>([deltaTime](TaskSystemsArgs& components)
	{
		if ((components.m_entity.IsKillable() && !components.m_entity.IsAlive()) || components.m_entity.IsPassenger())
		{
			return;
		}

		ProcessIdleEntity(components);
		ProcessInRangeOfTargetEntity(components);
	});
}

void TaskSystems::ProcessIdleEntity(const TaskSystemsArgs& components)
{
	ARGUS_TRACE(TaskSystems::ProcessIdleEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (!components.m_entity.IsIdle())
	{
		return;
	}

	if (components.m_navigationComponent && !components.m_navigationComponent->m_queuedWaypoints.IsEmpty())
	{
		NavigationSystems::StartNavigatingToQueuedWaypoint(components.m_taskComponent, components.m_targetingComponent, components.m_navigationComponent);
		return;
	}

	const bool hasConstructionTargets = components.m_nearbyEntitiesComponent->HasConstructionTargetsInSightRange();
	const bool hasCombatTargets = components.m_nearbyEntitiesComponent->HasCombatTargetsInSightRange();
	if (!hasConstructionTargets && !hasCombatTargets)
	{
		return;
	}

	components.m_nearbyEntitiesComponent->IterateSeenEntityIds(true, true, [&components, hasConstructionTargets, hasCombatTargets](uint16 entityId)
	{
		return ProcessDispatchingForEntityPair(components, entityId, hasConstructionTargets, hasCombatTargets);
	});
}

bool TaskSystems::ProcessDispatchingForEntityPair(const TaskSystemsArgs& components, uint16 potentialTargetEntityId, bool checkConstruction, bool checkCombat)
{
	ARGUS_TRACE(TaskSystems::ProcessDispatchingForEntityPair);

	ArgusEntity potentialTargetEntity = ArgusEntity::RetrieveEntity(potentialTargetEntityId);

	if (checkConstruction && DispatchToConstructionIfAble(components, potentialTargetEntity))
	{
		return true;
	}
	if (checkCombat&& DispatchToCombatIfAble(components, potentialTargetEntity))
	{
		return true;
	}

	return false;
}

bool TaskSystems::DispatchToConstructionIfAble(const TaskSystemsArgs& components, ArgusEntity potentialTargetEntity)
{
	ARGUS_TRACE(TaskSystems::DispatchToConstructionIfAble);

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

bool TaskSystems::DispatchToCombatIfAble(const TaskSystemsArgs& components, ArgusEntity potentialTargetEntity)
{
	ARGUS_TRACE(TaskSystems::DispatchToCombatIfAble);

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
	ARGUS_TRACE(TaskSystems::ProcessInRangeOfTargetEntity);

	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	if (components.m_taskComponent->m_movementState != EMovementState::InRangeOfTargetEntity)
	{
		return;
	}

	if (components.m_entity.IsInRangeOfTargetEntity())
	{
		return;
	}

	components.m_taskComponent->m_movementState = EMovementState::MoveToEntity;
}

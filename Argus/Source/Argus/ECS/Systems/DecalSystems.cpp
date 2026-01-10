// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DecalSystems.h"
#include "ArgusEntity.h"
#include "ArgusEntityTemplate.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "RecordDefinitions/ArgusActorRecord.h"

void DecalSystems::RunSystems(UWorld* worldPointer, float deltaTime)
{
	ARGUS_TRACE(DecalSystems::RunSystems);

	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);

	ArgusEntity::IterateSystemsArgs<DecalSystemsArgs>([worldPointer, deltaTime](DecalSystemsArgs& components)
	{
		if (components.m_decalComponent->m_lifetimeSeconds > 0.0f && !components.m_decalComponent->m_lifetimeTimer.WasTimerSet())
		{
			components.m_decalComponent->m_lifetimeTimer.StartTimer(components.m_decalComponent->m_lifetimeSeconds);
		}
		else if (components.m_decalComponent->m_lifetimeTimer.IsTimerComplete())
		{
			components.m_taskComponent->m_baseState = EBaseState::DestroyedWaitingForActorRelease;
		}

		if (components.m_decalComponent->m_connectedEntityId != ArgusECSConstants::k_maxEntities)
		{
			DrawLineForConnectedDecals(worldPointer, components);
		}
	});
}

void DecalSystems::DrawLineForConnectedDecals(UWorld* worldPointer, DecalSystemsArgs& components)
{
	ARGUS_RETURN_ON_NULL(worldPointer, ArgusECSLog);
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}

	FVector fromLocation = components.m_transformComponent->m_location;
	fromLocation.Z += ArgusECSConstants::k_connectedDecalLineHeightAdjustment;

	ArgusEntity connectedEntity = ArgusEntity::RetrieveEntity(components.m_decalComponent->m_connectedEntityId);
	if (!connectedEntity)
	{
		components.m_decalComponent->m_connectedEntityId = ArgusECSConstants::k_maxEntities;
		return;
	}

	TransformComponent* connectedTransformComponent = connectedEntity.GetComponent<TransformComponent>();
	if (!connectedTransformComponent)
	{
		return;
	}

	FVector toLocation = connectedTransformComponent->m_location;
	toLocation.Z += ArgusECSConstants::k_connectedDecalLineHeightAdjustment;

	DrawDebugLine(worldPointer, fromLocation, toLocation, FColor::Green);
}

ArgusEntity DecalSystems::InstantiateMoveToLocationDecalEntity(const UArgusActorRecord* moveToLocationDecalRecord, const FVector& targetLocation, uint16 numReferencers, uint16 connectedEntityId, bool onAttackMove)
{
	ARGUS_TRACE(DecalSystems::InstantiateMoveToLocationDecalEntity);

	if (!moveToLocationDecalRecord)
	{
		return ArgusEntity::k_emptyEntity;
	}

	const UArgusEntityTemplate* moveToLocationDecalTemplate = moveToLocationDecalRecord->m_entityTemplate.LoadAndStorePtr();
	if (!moveToLocationDecalTemplate)
	{
		return ArgusEntity::k_emptyEntity;
	}

	MaterialCacheComponent* materialCacheComponent = ArgusEntity::GetSingletonEntity().GetComponent<MaterialCacheComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(materialCacheComponent, ArgusUnrealObjectsLog, ArgusEntity::k_emptyEntity);
	if (onAttackMove)
	{
		materialCacheComponent->m_attackMoveToLocationDecalMaterial.AsyncPreLoadAndStorePtr();
	}
	else
	{
		materialCacheComponent->m_moveToLocationDecalMaterial.AsyncPreLoadAndStorePtr();
	}

	const uint32 recordId = moveToLocationDecalRecord->m_id;
	TFunction<void(ArgusEntity)> callback = nullptr;
	callback = [targetLocation, recordId, numReferencers, connectedEntityId, onAttackMove](ArgusEntity entity)
	{
		if (TransformComponent* decalTransformComponent = entity.GetComponent<TransformComponent>())
		{
			decalTransformComponent->m_location = targetLocation;
			decalTransformComponent->m_radius = 0.0f;
		}
		if (TaskComponent* decalTaskComponent = entity.GetComponent<TaskComponent>())
		{
			decalTaskComponent->m_spawnedFromArgusActorRecordId = recordId;
			decalTaskComponent->m_baseState = EBaseState::SpawnedWaitingForActorTake;
		}
		if (ArgusDecalComponent* decalDecalComponent = entity.GetComponent<ArgusDecalComponent>())
		{
			decalDecalComponent->m_referencingEntityCount = numReferencers;
			decalDecalComponent->m_connectedEntityId = connectedEntityId;
			decalDecalComponent->m_decalType = onAttackMove ? EDecalType::AttackMoveToLocation : EDecalType::MoveToLocation;
		}
	};

	return moveToLocationDecalTemplate->MakeEntityAsync(callback);
}

void DecalSystems::SetMoveToLocationDecalPerEntity(TargetingComponent* targetingComponent, ArgusEntity decalEntity)
{
	ARGUS_TRACE(DecalSystems::SetMoveToLocationDecalPerEntity);

	ARGUS_RETURN_ON_NULL(targetingComponent, ArgusInputLog);

	ArgusEntity oldDecalEntity = ArgusEntity::RetrieveEntity(targetingComponent->m_decalEntityId);
	if (oldDecalEntity)
	{
		ArgusDecalComponent* oldDecalComponent = oldDecalEntity.GetComponent<ArgusDecalComponent>();
		TaskComponent* oldTaskComponent = oldDecalEntity.GetComponent<TaskComponent>();
		ARGUS_RETURN_ON_NULL(oldDecalComponent, ArgusInputLog);
		ARGUS_RETURN_ON_NULL(oldTaskComponent, ArgusInputLog);

		oldDecalComponent->m_referencingEntityCount--;
		if (oldDecalComponent->m_referencingEntityCount == 0u)
		{
			oldTaskComponent->m_baseState = EBaseState::DestroyedWaitingForActorRelease;
		}
	}

	targetingComponent->m_decalEntityId = decalEntity.GetId();
}

void DecalSystems::ActivateCachedMoveToLocationDecalPerEntity(const UArgusActorRecord* moveToLocationDecalRecord, ArgusEntity entity)
{
	ARGUS_TRACE(DecalSystems::ActivateCachedMoveToLocationDecalPerEntity);

	if (!entity)
	{
		return;
	}

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	ARGUS_RETURN_ON_NULL(taskComponent, ArgusInputLog);

	if (taskComponent->m_movementState != EMovementState::MoveToLocation)
	{
		return;
	}

	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return;
	}

	if (targetingComponent->HasLocationTarget())
	{
		const FVector targetLocation = targetingComponent->m_targetLocation.GetValue();
		ActivateMoveToLocationDecalEntity(moveToLocationDecalRecord, targetLocation, ArgusECSConstants::k_maxEntities, targetingComponent->m_decalEntityId, taskComponent->m_combatState == ECombatState::OnAttackMove);
	}

	NavigationComponent* navigationComponent = entity.GetComponent<NavigationComponent>();
	if (!navigationComponent)
	{
		return;
	}

	uint16 previousEntityId = targetingComponent->m_decalEntityId;
	for (NavigationWaypoint& queuedWaypoint : navigationComponent->m_queuedWaypoints)
	{
		ActivateMoveToLocationDecalEntity(moveToLocationDecalRecord, queuedWaypoint.m_location, previousEntityId, queuedWaypoint.m_decalEntityId, false);
		previousEntityId = queuedWaypoint.m_decalEntityId;
	}
}

void DecalSystems::ClearMoveToLocationDecalPerEntity(ArgusEntity entity, bool clearQueuedWaypoints)
{
	ARGUS_TRACE(DecalSystems::ClearMoveToLocationDecalPerEntity);

	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return;
	}

	ClearMoveToLocationDecalEntity(targetingComponent->m_decalEntityId);

	if (!clearQueuedWaypoints)
	{
		return;
	}

	NavigationComponent* navigationComponent = entity.GetComponent<NavigationComponent>();
	if (!navigationComponent)
	{
		return;
	}

	for (NavigationWaypoint& queuedWaypoint : navigationComponent->m_queuedWaypoints)
	{
		ClearMoveToLocationDecalEntity(queuedWaypoint.m_decalEntityId);
	}
}

uint16 DecalSystems::GetMostRecentSelectedWaypointDecalEntityId()
{
	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(inputInterfaceComponent, ArgusECSLog, ArgusECSConstants::k_maxEntities);

	if (inputInterfaceComponent->m_selectedArgusEntityIds.IsEmpty())
	{
		return ArgusECSConstants::k_maxEntities;
	}

	ArgusEntity oneSelectedEntity = ArgusEntity::k_emptyEntity;
	for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		ArgusEntity potentialEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
		if (!potentialEntity || !potentialEntity.GetComponent<NavigationComponent>() || !potentialEntity.GetComponent<TargetingComponent>())
		{
			continue;
		}
		oneSelectedEntity = potentialEntity;
	}
	
	if (!oneSelectedEntity)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	NavigationComponent* navigationComponent = oneSelectedEntity.GetComponent<NavigationComponent>();
	if (!navigationComponent)
	{
		return ArgusECSConstants::k_maxEntities;
	}

	if (navigationComponent->m_queuedWaypoints.IsEmpty())
	{
		TargetingComponent* targetingComponent = oneSelectedEntity.GetComponent<TargetingComponent>();
		if (!targetingComponent || !targetingComponent->HasLocationTarget())
		{
			return ArgusECSConstants::k_maxEntities;
		}

		return targetingComponent->m_decalEntityId;
	}

	return navigationComponent->m_queuedWaypoints.Last().m_decalEntityId;
}

void DecalSystems::ActivateMoveToLocationDecalEntity(const UArgusActorRecord* moveToLocationDecalRecord, const FVector& location, uint16 connectedEntityId, uint16& decalEntityId, bool newSelectedOnAttackMove)
{
	ARGUS_TRACE(DecalSystems::ActivateMoveToLocationDecalEntity);

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	bool priorSelectedOnAttackMove = false;
	for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		ArgusEntity selectedEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
		if (!selectedEntity)
		{
			continue;
		}

		TargetingComponent* selectedTargetingComponent = selectedEntity.GetComponent<TargetingComponent>();
		if (!selectedTargetingComponent || !selectedTargetingComponent->HasLocationTarget())
		{
			continue;
		}

		if (selectedTargetingComponent->m_targetLocation.GetValue() == location)
		{
			decalEntityId = selectedTargetingComponent->m_decalEntityId;
			ArgusEntity decalEntity = ArgusEntity::RetrieveEntity(selectedTargetingComponent->m_decalEntityId);
			if (decalEntity)
			{
				if (ArgusDecalComponent* decalComponent = decalEntity.GetComponent<ArgusDecalComponent>())
				{
					decalComponent->m_referencingEntityCount++;
					decalComponent->m_connectedEntityId = connectedEntityId;
				}
			}

			if (TaskComponent* taskComponent = selectedEntity.GetComponent<TaskComponent>())
			{
				priorSelectedOnAttackMove |= taskComponent->m_combatState == ECombatState::OnAttackMove;
			}
			return;
		}

		NavigationComponent* navigationComponent = selectedEntity.GetComponent<NavigationComponent>();
		if (!navigationComponent)
		{
			continue;
		}

		for (NavigationWaypoint& queuedWaypoint : navigationComponent->m_queuedWaypoints)
		{
			if (queuedWaypoint.m_location != location)
			{
				continue;
			}

			decalEntityId = queuedWaypoint.m_decalEntityId;
			ArgusEntity decalEntity = ArgusEntity::RetrieveEntity(decalEntityId);
			if (decalEntity)
			{
				if (ArgusDecalComponent* decalComponent = decalEntity.GetComponent<ArgusDecalComponent>())
				{
					decalComponent->m_referencingEntityCount++;
					decalComponent->m_connectedEntityId = connectedEntityId;
				}
			}

			return;
		}
	}

	decalEntityId = DecalSystems::InstantiateMoveToLocationDecalEntity(moveToLocationDecalRecord, location, 1u, connectedEntityId, newSelectedOnAttackMove || priorSelectedOnAttackMove).GetId();
}

void DecalSystems::ClearMoveToLocationDecalEntity(uint16& decalEntityId)
{
	ARGUS_TRACE(DecalSystems::ClearMoveToLocationDecalEntity);

	ArgusEntity oldDecalEntity = ArgusEntity::RetrieveEntity(decalEntityId);
	decalEntityId = ArgusECSConstants::k_maxEntities;

	if (!oldDecalEntity)
	{
		return;
	}

	ArgusDecalComponent* decalComponent = oldDecalEntity.GetComponent<ArgusDecalComponent>();
	TaskComponent* taskComponent = oldDecalEntity.GetComponent<TaskComponent>();
	if (!decalComponent || !taskComponent)
	{
		return;
	}

	decalComponent->m_referencingEntityCount--;
	if (decalComponent->m_referencingEntityCount == 0u)
	{
		taskComponent->m_baseState = EBaseState::DestroyedWaitingForActorRelease;
	}
}

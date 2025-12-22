// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "InputInterfaceSystems.h"
#include "ArgusActor.h"
#include "ArgusGameInstance.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "Systems/AvoidanceSystems.h"
#include "Systems/DecalSystems.h"

void InputInterfaceSystems::MoveSelectedEntitiesToTarget(EMovementState inputMovementState, ArgusEntity targetEntity, const FVector& targetLocation, ArgusEntity decalEntity)
{
	IterateSelectedEntities([inputMovementState, targetEntity, targetLocation, decalEntity](ArgusEntity selectedEntity)
	{
		MoveEntityToTarget(selectedEntity, inputMovementState, targetEntity, targetLocation, decalEntity);
	});
}

void InputInterfaceSystems::SetWaypointForSelectedEntities(const FVector& targetLocation, ArgusEntity decalEntity)
{
	IterateSelectedEntities([targetLocation, decalEntity](ArgusEntity selectedEntity)
	{
		SetWaypointForEntity(selectedEntity, targetLocation, decalEntity);
	});
}

uint16 InputInterfaceSystems::GetNumWaypointEligibleEntities()
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(inputInterfaceComponent, ArgusECSLog, 0u);

	uint16 numWaypointEligibleEntities = 0u;
	IterateSelectedEntities([&numWaypointEligibleEntities](ArgusEntity selectedEntity) 
	{
		if (!selectedEntity.GetComponent<TaskComponent>() || !selectedEntity.GetComponent<TargetingComponent>() || !selectedEntity.GetComponent<NavigationComponent>())
		{
			return;
		}

		numWaypointEligibleEntities++;
	});

	return numWaypointEligibleEntities;
}

void InputInterfaceSystems::AddSelectedEntityExclusive(ArgusEntity selectedEntity, const UArgusActorRecord* moveToLocationDecalActorRecord)
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	const UArgusGameInstance* gameInstance = UArgusGameInstance::GetArgusGameInstance();
	ARGUS_RETURN_ON_NULL(gameInstance, ArgusECSLog);

	AArgusActor* actor = gameInstance->GetArgusActorFromArgusEntity(selectedEntity);
	ARGUS_RETURN_ON_NULL(actor, ArgusECSLog);

	bool alreadySelected = RemoveAllSelectedEntities(selectedEntity);
	if (!alreadySelected)
	{
		actor->SetSelectionState(true);
		DecalSystems::ActivateCachedMoveToLocationDecalPerEntity(moveToLocationDecalActorRecord, selectedEntity);
	}
	inputInterfaceComponent->m_selectedArgusEntityIds.Add(selectedEntity.GetId());

	InputInterfaceSystems::CheckAndHandleEntityDoubleClick(selectedEntity, moveToLocationDecalActorRecord);

	OnSelectedEntitiesChanged();
}

void InputInterfaceSystems::AddSelectedEntityAdditive(ArgusEntity selectedEntity, const UArgusActorRecord* moveToLocationDecalActorRecord)
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	const UArgusGameInstance* gameInstance = UArgusGameInstance::GetArgusGameInstance();
	ARGUS_RETURN_ON_NULL(gameInstance, ArgusECSLog);

	AArgusActor* actor = gameInstance->GetArgusActorFromArgusEntity(selectedEntity);
	ARGUS_RETURN_ON_NULL(actor, ArgusECSLog);

	if (inputInterfaceComponent->m_selectedArgusEntityIds.Contains(selectedEntity.GetId()))
	{
		actor->SetSelectionState(false);
		DecalSystems::ClearMoveToLocationDecalPerEntity(selectedEntity, true);
		inputInterfaceComponent->m_selectedArgusEntityIds.Remove(selectedEntity.GetId());
	}
	else
	{
		actor->SetSelectionState(true);
		DecalSystems::ActivateCachedMoveToLocationDecalPerEntity(moveToLocationDecalActorRecord, selectedEntity);
		inputInterfaceComponent->m_selectedArgusEntityIds.Add(selectedEntity.GetId());
	}

	OnSelectedEntitiesChanged();
}

void InputInterfaceSystems::AddMultipleSelectedEntitiesExclusive(TArray<uint16>& selectedEntityIds, const UArgusActorRecord* moveToLocationDecalActorRecord)
{
	RemoveAllSelectedEntities(ArgusEntity::k_emptyEntity);
	AddMultipleSelectedEntitiesAdditive(selectedEntityIds, moveToLocationDecalActorRecord);
}

void InputInterfaceSystems::RemoveNoLongerSelectableEntities()
{
	const UArgusGameInstance* gameInstance = UArgusGameInstance::GetArgusGameInstance();
	ARGUS_RETURN_ON_NULL(gameInstance, ArgusECSLog);

	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	bool didRemoveEntities = false;

	TArray<ArgusEntity> entitiesToRemove;
	IterateSelectedEntities([&entitiesToRemove](ArgusEntity selectedEntity)
	{
		if (!selectedEntity.IsAlive() || selectedEntity.IsPassenger())
		{
			entitiesToRemove.Add(selectedEntity);
		}
	});

	for (int32 i = 0; i < entitiesToRemove.Num(); ++i)
	{
		didRemoveEntities = true;
		RemoveSelectionStateForEntity(entitiesToRemove[i]);
		inputInterfaceComponent->m_selectedArgusEntityIds.Remove(entitiesToRemove[i].GetId());
	}

	if (didRemoveEntities)
	{
		OnSelectedEntitiesChanged();
	}
}

bool InputInterfaceSystems::RemoveAllSelectedEntities(ArgusEntity excludedEntity)
{
	bool wasExcludedEntityPresent = false;

	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusInputLog);

	IterateSelectedEntities([&wasExcludedEntityPresent, excludedEntity](ArgusEntity selectedEntity)
	{
		if (selectedEntity.GetId() != excludedEntity.GetId())
		{
			RemoveSelectionStateForEntity(selectedEntity);
		}
		else
		{
			wasExcludedEntityPresent = true;
		}
	});

	inputInterfaceComponent->m_selectedArgusEntityIds.Reset();
	inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Reset();
	inputInterfaceComponent->m_indexOfActiveAbilityGroup = -1;
	inputInterfaceComponent->m_selectedActorsDisplayState = ESelectedActorsDisplayState::ChangedThisFrame;

	return wasExcludedEntityPresent;
}

void InputInterfaceSystems::SetAbilityStateForCastIndex(uint8 abilityIndex)
{
	IterateActiveAbilityGroupEntities([abilityIndex](ArgusEntity selectedEntity)
	{
		TaskComponent* taskComponent = selectedEntity.GetComponent<TaskComponent>();
		if (!taskComponent)
		{
			return;
		}

		switch (abilityIndex)
		{
			case 0u:
				taskComponent->m_abilityState = EAbilityState::ProcessCastAbility0Command;
				break;
			case 1u:
				taskComponent->m_abilityState = EAbilityState::ProcessCastAbility1Command;
				break;
			case 2u:
				taskComponent->m_abilityState = EAbilityState::ProcessCastAbility2Command;
				break;
			case 3u:
				taskComponent->m_abilityState = EAbilityState::ProcessCastAbility3Command;
				break;
			default:
				break;
		}
	});
}

void InputInterfaceSystems::SetAbilityStateForReticleAbility(const ReticleComponent* reticleComponent)
{
	ARGUS_RETURN_ON_NULL(reticleComponent, ArgusECSLog);

	IterateActiveAbilityGroupEntities([reticleComponent](ArgusEntity selectedEntity)
	{
		TaskComponent* taskComponent = selectedEntity.GetComponent<TaskComponent>();
		if (!taskComponent)
		{
			return;
		}

		AbilityComponent* abilityComponent = selectedEntity.GetComponent<AbilityComponent>();
		if (!abilityComponent)
		{
			return;
		}

		if (!abilityComponent->HasAbility(reticleComponent->m_abilityRecordId))
		{
			return;
		}

		taskComponent->m_abilityState = EAbilityState::ProcessCastReticleAbility;
	});
}

void InputInterfaceSystems::ChangeActiveAbilityGroup()
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusInputLog);

	if (inputInterfaceComponent->m_selectedArgusEntityIds.Num() == 0)
	{
		return;
	}

	const int8 previousIndexOfActiveAbilityGroup = inputInterfaceComponent->m_indexOfActiveAbilityGroup;
	if (previousIndexOfActiveAbilityGroup < 0)
	{
		return;
	}

	const AbilityComponent* previousActiveAbilityGroupAbilities = nullptr;
	if (ArgusEntity previousTemplateEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[previousIndexOfActiveAbilityGroup]))
	{
		previousActiveAbilityGroupAbilities = previousTemplateEntity.GetComponent<AbilityComponent>();
	}

	const AbilityComponent* templateEntityAbilities = nullptr;
	for (int8 i = 1; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		int8 indexToCheck = ((previousIndexOfActiveAbilityGroup + i) % inputInterfaceComponent->m_selectedArgusEntityIds.Num());

		ArgusEntity entityToCheck = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[indexToCheck]);
		if (!entityToCheck)
		{
			continue;
		}

		const AbilityComponent* abilityComponentToCheck = entityToCheck.GetComponent<AbilityComponent>();
		if (!abilityComponentToCheck)
		{
			continue;
		}

		if (previousActiveAbilityGroupAbilities && previousActiveAbilityGroupAbilities->HasSameAbilities(abilityComponentToCheck))
		{
			continue;
		}

		if (templateEntityAbilities == nullptr)
		{
			templateEntityAbilities = abilityComponentToCheck;
			inputInterfaceComponent->m_indexOfActiveAbilityGroup = indexToCheck;
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Empty();
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(entityToCheck.GetId());
			inputInterfaceComponent->m_selectedActorsDisplayState = ESelectedActorsDisplayState::ChangedThisFrame;
			continue;
		}

		if (templateEntityAbilities->HasSameAbilities(abilityComponentToCheck))
		{
			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(entityToCheck.GetId());
		}
	}
}

void InputInterfaceSystems::SetControlGroup(uint8 controlGroupIndex)
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusInputLog);

	if (controlGroupIndex >= inputInterfaceComponent->m_numControlGroups)
	{
		return;
	}

	if (inputInterfaceComponent->m_controlGroups[controlGroupIndex].Max() < inputInterfaceComponent->m_selectedArgusEntityIds.Num())
	{
		inputInterfaceComponent->m_controlGroups[controlGroupIndex].Reserve(inputInterfaceComponent->m_selectedArgusEntityIds.Num());
	}
	inputInterfaceComponent->m_controlGroups[controlGroupIndex].Reset();

	for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
	{
		inputInterfaceComponent->m_controlGroups[controlGroupIndex].Add(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
	}
}

void InputInterfaceSystems::SelectControlGroup(uint8 controlGroupIndex, const UArgusActorRecord* moveToLocationDecalActorRecord)
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusInputLog);

	if (controlGroupIndex >= inputInterfaceComponent->m_numControlGroups)
	{
		return;
	}

	RemoveAllSelectedEntities(ArgusEntity::k_emptyEntity);
	AddMultipleSelectedEntitiesAdditive(inputInterfaceComponent->m_controlGroups[controlGroupIndex].m_entityIds, moveToLocationDecalActorRecord);
}

ArgusEntity InputInterfaceSystems::GetASelectedEntity()
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL_VALUE(inputInterfaceComponent, ArgusInputLog, ArgusEntity::k_emptyEntity);

	if (inputInterfaceComponent->m_selectedArgusEntityIds.Num() <= 0)
	{
		return ArgusEntity::k_emptyEntity;
	}

	return ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[0]);
}

void InputInterfaceSystems::CheckAndHandleEntityDoubleClick(ArgusEntity entity, const UArgusActorRecord* moveToLocationDecalActorRecord)
{
	if (!entity)
	{
		return;
	}

	ArgusEntity singletonEntity = ArgusEntity::GetSingletonEntity();
	InputInterfaceComponent* inputInterfaceComponent = singletonEntity.GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	if (inputInterfaceComponent->m_doubleClickTimer.IsTimerTicking(singletonEntity))
	{
		if (entity.GetId() == inputInterfaceComponent->m_lastSelectedEntityId)
		{
			inputInterfaceComponent->m_lastSelectedEntityId = ArgusECSConstants::k_maxEntities;
			inputInterfaceComponent->m_doubleClickTimer.CancelTimer(singletonEntity);

			AddAdjacentLikeEntitiesAsSelected(entity, inputInterfaceComponent, moveToLocationDecalActorRecord);
			return;
		}
	}

	inputInterfaceComponent->m_lastSelectedEntityId = entity.GetId();
	inputInterfaceComponent->m_doubleClickTimer.CancelTimer(singletonEntity);
	inputInterfaceComponent->m_doubleClickTimer.StartTimer(singletonEntity, inputInterfaceComponent->m_doubleClickThresholdSeconds);
}

void InputInterfaceSystems::InterruptReticle()
{
	ReticleComponent* reticleComponent = ArgusEntity::GetSingletonEntity().GetComponent<ReticleComponent>();
	ARGUS_RETURN_ON_NULL(reticleComponent, ArgusInputLog);

	if (!reticleComponent->IsReticleEnabled())
	{
		return;
	}

	reticleComponent->DisableReticle();
}

void InputInterfaceSystems::MoveEntityToTarget(ArgusEntity entity, EMovementState inputMovementState, ArgusEntity targetEntity, const FVector& targetLocation, ArgusEntity decalEntity)
{
	if (!entity)
	{
		return;
	}

	DecalSystems::ClearMoveToLocationDecalPerEntity(entity, true);

	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	NavigationComponent* navigationComponent = entity.GetComponent<NavigationComponent>();

	if (!taskComponent || !targetingComponent)
	{
		return;
	}

	if (navigationComponent)
	{
		navigationComponent->ResetQueuedWaypoints();
	}

	if (inputMovementState == EMovementState::ProcessMoveToEntityCommand)
	{
		if (targetEntity == entity)
		{
			taskComponent->m_movementState = EMovementState::None;
		}
		else
		{
			if (navigationComponent)
			{
				AvoidanceSystems::DecrementIdleEntitiesInGroup(entity);
				taskComponent->m_movementState = inputMovementState;
			}

			targetingComponent->m_targetEntityId = targetEntity.GetId();
			targetingComponent->m_targetLocation.Reset();
		}

		DecalSystems::ClearMoveToLocationDecalPerEntity(entity, true);
	}
	else if (inputMovementState == EMovementState::ProcessMoveToLocationCommand)
	{
		if (navigationComponent)
		{
			AvoidanceSystems::DecrementIdleEntitiesInGroup(entity);
			taskComponent->m_movementState = inputMovementState;
		}

		DecalSystems::SetMoveToLocationDecalPerEntity(targetingComponent, decalEntity);

		targetingComponent->m_targetEntityId = ArgusEntity::k_emptyEntity.GetId();
		targetingComponent->m_targetLocation = targetLocation;
	}
}

void InputInterfaceSystems::SetWaypointForEntity(ArgusEntity entity, const FVector& targetLocation, ArgusEntity decalEntity)
{
	TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	NavigationComponent* navigationComponent = entity.GetComponent<NavigationComponent>();

	if (!taskComponent || !targetingComponent || !navigationComponent)
	{
		return;
	}

	switch (taskComponent->m_movementState)
	{
		case EMovementState::None:
		case EMovementState::FailedToFindPath:
			taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
			targetingComponent->m_targetEntityId = ArgusEntity::k_emptyEntity.GetId();
			targetingComponent->m_targetLocation = targetLocation;
			targetingComponent->m_decalEntityId = decalEntity.GetId();
			navigationComponent->ResetQueuedWaypoints();
			break;
		case EMovementState::ProcessMoveToLocationCommand:
		case EMovementState::MoveToLocation:
			taskComponent->m_movementState = EMovementState::ProcessMoveToLocationCommand;
			navigationComponent->m_queuedWaypoints.PushLast(NavigationWaypoint(targetLocation, decalEntity.GetId()));
			break;
		default:
			navigationComponent->m_queuedWaypoints.PushLast(NavigationWaypoint(targetLocation, decalEntity.GetId()));
			break;
	}
}

void InputInterfaceSystems::RemoveSelectionStateForEntity(ArgusEntity entity)
{
	const UArgusGameInstance* gameInstance = UArgusGameInstance::GetArgusGameInstance();
	ARGUS_RETURN_ON_NULL(gameInstance, ArgusECSLog);

	AArgusActor* actor = gameInstance->GetArgusActorFromArgusEntity(entity);
	ARGUS_RETURN_ON_NULL(actor, ArgusECSLog);

	actor->SetSelectionState(false);
	DecalSystems::ClearMoveToLocationDecalPerEntity(entity, true);
}

void InputInterfaceSystems::AddAdjacentLikeEntitiesAsSelected(ArgusEntity entity, InputInterfaceComponent* inputInterfaceComponent, const UArgusActorRecord* moveToLocationDecalActorRecord)
{
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

	SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const TaskComponent* taskComponent = entity.GetComponent<TaskComponent>();
	const TransformComponent* transformComponent = entity.GetComponent<TransformComponent>();
	if (!taskComponent && !transformComponent)
	{
		return;
	}

	TFunction<bool(const ArgusEntityKDTreeNode*)> predicate = [entity, taskComponent, inputInterfaceComponent](const ArgusEntityKDTreeNode* node)
	{
		ARGUS_RETURN_ON_NULL_BOOL(taskComponent, ArgusECSLog);
		ARGUS_RETURN_ON_NULL_BOOL(inputInterfaceComponent, ArgusECSLog);
		if (!node)
		{
			return false;
		}

		ArgusEntity foundEntity = ArgusEntity::RetrieveEntity(node->m_entityId);
		if (!foundEntity || inputInterfaceComponent->IsEntityIdSelected(node->m_entityId))
		{
			return false;
		}

		const TaskComponent* foundTaskComponent = foundEntity.GetComponent<TaskComponent>();
		if (!foundTaskComponent)
		{
			return false;
		}

		if (!foundEntity.IsOnSameTeamAsOtherEntity(entity))
		{
			return false;
		}

		return foundTaskComponent->m_spawnedFromArgusActorRecordId == taskComponent->m_spawnedFromArgusActorRecordId;
	};

	TArray<uint16> foundFlyingEntityIds;
	if (transformComponent->CanFly())
	{
		spatialPartitioningComponent->m_flyingArgusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(foundFlyingEntityIds, transformComponent->m_location, inputInterfaceComponent->m_doubleClickQueryRange, predicate);
	}

	TArray<uint16> foundGroundedEntityIds;
	if (transformComponent->CanWalk())
	{
		spatialPartitioningComponent->m_argusEntityKDTree.FindArgusEntityIdsWithinRangeOfLocation(foundGroundedEntityIds, transformComponent->m_location, inputInterfaceComponent->m_doubleClickQueryRange, predicate);
	}

	if (foundGroundedEntityIds.GetSlack() < foundFlyingEntityIds.Num())
	{
		foundGroundedEntityIds.Reserve(foundGroundedEntityIds.Num() + foundFlyingEntityIds.Num());
	}
	for (int32 i = 0; i < foundFlyingEntityIds.Num(); ++i)
	{
		foundGroundedEntityIds.Add(foundFlyingEntityIds[i]);
	}

	AddMultipleSelectedEntitiesAdditive(foundGroundedEntityIds, moveToLocationDecalActorRecord);
}

void InputInterfaceSystems::OnSelectedEntitiesChanged()
{
	InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusInputLog);

	inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Reset();
	inputInterfaceComponent->m_indexOfActiveAbilityGroup = -1;
	inputInterfaceComponent->m_selectedActorsDisplayState = ESelectedActorsDisplayState::ChangedThisFrame;

	inputInterfaceComponent->m_selectedArgusEntityIds.Sort([](uint16 entityAId, uint16 entityBId)
	{
		const TaskComponent* taskComponentA = ArgusEntity::RetrieveEntity(entityAId).GetComponent<TaskComponent>();
		const TaskComponent* taskComponentB = ArgusEntity::RetrieveEntity(entityBId).GetComponent<TaskComponent>();
		ARGUS_RETURN_ON_NULL_BOOL(taskComponentA, ArgusInputLog);
		ARGUS_RETURN_ON_NULL_BOOL(taskComponentB, ArgusInputLog);

		const AbilityComponent* abilityComponentA = ArgusEntity::RetrieveEntity(entityAId).GetComponent<AbilityComponent>();
		const AbilityComponent* abilityComponentB = ArgusEntity::RetrieveEntity(entityBId).GetComponent<AbilityComponent>();

		const bool noAbilityA = !abilityComponentA || !abilityComponentA->HasAnyAbility();
		const bool noAbilityB = !abilityComponentB || !abilityComponentB->HasAnyAbility();
		if (noAbilityA || noAbilityB)
		{
			if (noAbilityA && noAbilityB)
			{
				return taskComponentA->m_spawnedFromArgusActorRecordId < taskComponentB->m_spawnedFromArgusActorRecordId;
			}

			return noAbilityB;
		}

		return abilityComponentA->m_abilityCasterPriority > abilityComponentB->m_abilityCasterPriority;
	});

	InterruptReticle();
	const int32 numSelected = inputInterfaceComponent->m_selectedArgusEntityIds.Num();
	if (numSelected == 0)
	{
		return;
	}

	inputInterfaceComponent->m_indexOfActiveAbilityGroup = 0;
	inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(inputInterfaceComponent->m_selectedArgusEntityIds[0]);

	const ArgusEntity templateEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[0]);
	const TaskComponent* templateTaskComponent = templateEntity.GetComponent<TaskComponent>();
	ARGUS_RETURN_ON_NULL(templateTaskComponent, ArgusInputLog);

	const AbilityComponent* templateAbilityComponent = templateEntity.GetComponent<AbilityComponent>();
	if (!templateAbilityComponent || !templateAbilityComponent->HasAnyAbility())
	{
		for (int32 i = 1; i < numSelected; ++i)
		{
			const TaskComponent* taskComponent = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]).GetComponent<TaskComponent>();
			if (!taskComponent || taskComponent->m_spawnedFromArgusActorRecordId != templateTaskComponent->m_spawnedFromArgusActorRecordId)
			{
				break;
			}

			inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
		}
		return;
	}

	for (int32 i = 1; i < numSelected; ++i)
	{
		const AbilityComponent* abilityComponent = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]).GetComponent<AbilityComponent>();
		if (!abilityComponent || !abilityComponent->HasAnyAbility() || !abilityComponent->HasSameAbilities(templateAbilityComponent))
		{
			break;
		}

		inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Add(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
	}
}
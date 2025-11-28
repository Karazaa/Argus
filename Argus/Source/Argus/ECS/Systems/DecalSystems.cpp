// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "DecalSystems.h"
#include "ArgusEntity.h"
#include "ArgusEntityTemplate.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "RecordDefinitions/ArgusActorRecord.h"

void DecalSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(DecalSystems::RunSystems);

	ArgusEntity::IterateSystemsArgs<DecalSystemsArgs>([deltaTime](DecalSystemsArgs& components)
	{
		if (components.m_decalComponent->m_lifetimeSeconds > 0.0f && !components.m_decalComponent->m_lifetimeTimer.WasTimerSet())
		{
			components.m_decalComponent->m_lifetimeTimer.StartTimer(components.m_decalComponent->m_lifetimeSeconds);
			return;
		}

		if (components.m_decalComponent->m_lifetimeTimer.IsTimerComplete())
		{
			components.m_taskComponent->m_baseState = EBaseState::DestroyedWaitingForActorRelease;
		}
	});
}

ArgusEntity DecalSystems::InstantiateMoveToLocationDecalEntity(const UArgusActorRecord* moveToLocationDecalRecord, const FVector& targetLocation, uint16 numReferencers)
{
	if (!moveToLocationDecalRecord)
	{
		return ArgusEntity::k_emptyEntity;
	}

	const UArgusEntityTemplate* moveToLocationDecalTemplate = moveToLocationDecalRecord->m_entityTemplate.LoadAndStorePtr();
	if (!moveToLocationDecalTemplate)
	{
		return ArgusEntity::k_emptyEntity;
	}

	const uint32 recordId = moveToLocationDecalRecord->m_id;
	TFunction<void(ArgusEntity)> callback = nullptr;
	callback = [targetLocation, recordId, numReferencers](ArgusEntity entity)
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
		if (DecalComponent* decalDecalComponent = entity.GetComponent<DecalComponent>())
		{
			decalDecalComponent->m_referencingEntityCount = numReferencers;
		}
	};

	return moveToLocationDecalTemplate->MakeEntityAsync(callback);
}

void DecalSystems::SetMoveToLocationDecalPerEntity(TargetingComponent* targetingComponent, ArgusEntity decalEntity)
{
	ARGUS_RETURN_ON_NULL(targetingComponent, ArgusInputLog);

	ArgusEntity oldDecalEntity = ArgusEntity::RetrieveEntity(targetingComponent->m_decalEntityId);
	if (oldDecalEntity)
	{
		DecalComponent* oldDecalComponent = oldDecalEntity.GetComponent<DecalComponent>();
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

	if (!targetingComponent->HasLocationTarget())
	{
		return;
	}

	const FVector targetLocation = targetingComponent->m_targetLocation.GetValue();

	const InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
	ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);
	
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

		if (selectedTargetingComponent->m_targetLocation.GetValue() == targetLocation)
		{
			targetingComponent->m_decalEntityId = selectedTargetingComponent->m_decalEntityId;
			ArgusEntity decalEntity = ArgusEntity::RetrieveEntity(selectedTargetingComponent->m_decalEntityId);
			if (decalEntity)
			{
				if (DecalComponent* decalComponent = decalEntity.GetComponent<DecalComponent>())
				{
					decalComponent->m_referencingEntityCount++;
				}
			}

			return;
		}
	}

	targetingComponent->m_decalEntityId = DecalSystems::InstantiateMoveToLocationDecalEntity(moveToLocationDecalRecord, targetLocation, 1u).GetId();
}

void DecalSystems::ClearMoveToLocationDecalPerEntity(ArgusEntity entity)
{
	TargetingComponent* targetingComponent = entity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		return;
	}

	ArgusEntity oldDecalEntity = ArgusEntity::RetrieveEntity(targetingComponent->m_decalEntityId);
	targetingComponent->m_decalEntityId = ArgusECSConstants::k_maxEntities;
	if (!oldDecalEntity)
	{
		return;
	}

	DecalComponent* decalComponent = oldDecalEntity.GetComponent<DecalComponent>();
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

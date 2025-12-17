// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class UArgusActorRecord;

struct InputInterfaceComponent;

class InputInterfaceSystems
{
public:
	static void MoveSelectedEntitiesToTarget(EMovementState inputMovementState, ArgusEntity targetEntity, const FVector& targetLocation, ArgusEntity decalEntity);
	static void SetWaypointForSelectedEntities(const FVector& targetLocation, ArgusEntity decalEntity);
	static uint16 GetNumWaypointEligibleEntities();

	static void AddSelectedEntityExclusive(ArgusEntity selectedEntity, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static void AddSelectedEntityAdditive(ArgusEntity selectedEntity, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static void AddMultipleSelectedEntitiesExclusive(TArray<uint16>& selectedEntityIds, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static void AddMultipleSelectedEntitiesAdditive(TArray<uint16>& selectedEntityIds, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static void RemoveNoLongerSelectableEntities();
	static bool RemoveAllSelectedEntities(ArgusEntity excludedEntity);

	static void SetControlGroup(uint8 controlGroupIndex);
	static void SelectControlGroup(uint8 controlGroupIndex, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static ArgusEntity GetASelectedEntity();

	static void CheckAndHandleEntityDoubleClick(ArgusEntity entity);

	static void InterruptReticle();

private:
	static void MoveEntityToTarget(ArgusEntity entity, EMovementState inputMovementState, ArgusEntity targetEntity, const FVector& targetLocation, ArgusEntity decalEntity);
	static void SetWaypointForEntity(ArgusEntity entity, const FVector& targetLocation, ArgusEntity decalEntity);
	static void RemoveSelectionStateForEntity(ArgusEntity entity);
	static void AddAdjacentLikeEntitiesAsSelected(ArgusEntity entity, InputInterfaceComponent* inputInterfaceComponent);
	static void AddEntityIdAsSelected(uint16 entityId);

	static void OnSelectedEntitiesChanged();

	template <typename Function>
	static void IterateSelectedEntities(Function&& perEntityFunction)
	{
		InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
		ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

		for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
		{
			ArgusEntity selectedEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
			perEntityFunction(selectedEntity);
		}
	}
};

// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class UArgusActorRecord;

struct InputInterfaceComponent;

class InputInterfaceSystems
{
public:
	static bool HasAnySelectedEntities();
	static void MoveSelectedEntitiesToTarget(EMovementState inputMovementState, ArgusEntity targetEntity, const FVector& targetLocation, ArgusEntity decalEntity, bool onAttackMove);
	static void SetWaypointForSelectedEntities(const FVector& targetLocation, ArgusEntity decalEntity);
	static uint16 GetNumWaypointEligibleEntities();

	static void AddSelectedEntityExclusive(ArgusEntity selectedEntity, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static void AddSelectedEntityAdditive(ArgusEntity selectedEntity, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static void AddMultipleSelectedEntitiesExclusive(TArray<uint16>& selectedEntityIds, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static void RemoveNoLongerSelectableEntities();
	static bool RemoveAllSelectedEntities(ArgusEntity excludedEntity);
	static void SetAbilityStateForCastIndex(uint8 abilityIndex);
	static void SetAbilityStateForReticleAbility(const ReticleComponent* reticleComponent);
	static void ChangeActiveAbilityGroup();

	static void SetControlGroup(uint8 controlGroupIndex);
	static void SelectControlGroup(uint8 controlGroupIndex, const UArgusActorRecord* moveToLocationDecalActorRecord);
	static ArgusEntity GetASelectedEntity();

	static void CheckAndHandleEntityDoubleClick(ArgusEntity entity, const UArgusActorRecord* moveToLocationDecalActorRecord);

	static void InterruptReticle();

	template <typename Array>
	static void AddMultipleSelectedEntitiesAdditive(Array& selectedEntityIds, const UArgusActorRecord* moveToLocationDecalActorRecord)
	{
		InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
		ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

		const int32 numSelectedEntities = selectedEntityIds.Num();
		if (numSelectedEntities > inputInterfaceComponent->m_selectedArgusEntityIds.GetSlack())
		{
			inputInterfaceComponent->m_selectedArgusEntityIds.Reserve(selectedEntityIds.Num() + inputInterfaceComponent->m_selectedArgusEntityIds.Num());
		}

		for (int32 i = 0; i < numSelectedEntities; ++i)
		{
			ArgusEntity selectedEntity = ArgusEntity::RetrieveEntity(selectedEntityIds[i]);
			if (selectedEntityIds[i] && !inputInterfaceComponent->m_selectedArgusEntityIds.Contains(selectedEntityIds[i]))
			{
				AddSelectedEntityAdditive(selectedEntity, moveToLocationDecalActorRecord);
			}
		}

		if (numSelectedEntities > 0)
		{
			OnSelectedEntitiesChanged();
		}
	}

	template <typename Function>
	static void IterateSelectedEntities(Function&& perEntityFunction)
	{
		InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
		ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

		for (int32 i = 0; i < inputInterfaceComponent->m_selectedArgusEntityIds.Num(); ++i)
		{
			ArgusEntity selectedEntity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_selectedArgusEntityIds[i]);
			if (selectedEntity)
			{
				perEntityFunction(selectedEntity);
			}
		}
	}

	template <typename Function>
	static void IterateActiveAbilityGroupEntities(Function&& perEntityFunction)
	{
		InputInterfaceComponent* inputInterfaceComponent = ArgusEntity::GetSingletonEntity().GetComponent<InputInterfaceComponent>();
		ARGUS_RETURN_ON_NULL(inputInterfaceComponent, ArgusECSLog);

		for (int32 i = 0; i < inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds.Num(); ++i)
		{
			ArgusEntity entity = ArgusEntity::RetrieveEntity(inputInterfaceComponent->m_activeAbilityGroupArgusEntityIds[i]);
			if (entity)
			{
				perEntityFunction(entity);
			}
		}
	}

private:
	static void MoveEntityToTarget(ArgusEntity entity, EMovementState inputMovementState, ArgusEntity targetEntity, const FVector& targetLocation, ArgusEntity decalEntity, bool onAttackMove);
	static void SetWaypointForEntity(ArgusEntity entity, const FVector& targetLocation, ArgusEntity decalEntity);
	static void RemoveSelectionStateForEntity(ArgusEntity entity);
	static void AddAdjacentLikeEntitiesAsSelected(ArgusEntity entity, InputInterfaceComponent* inputInterfaceComponent, const UArgusActorRecord* moveToLocationDecalActorRecord);

	static void OnSelectedEntitiesChanged();
};

// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ComponentData.h"
#include "TaskComponentData.generated.h"

UCLASS()
class ARGUS_API UTaskComponentData : public UComponentData
{
	GENERATED_BODY()

public:
	uint32 m_spawnedFromArgusActorRecordId = 0u;
	BaseState m_baseState = BaseState::None;
	MovementState m_movementState = MovementState::None;
	SpawningState m_spawningState = SpawningState::None;
	AbilityState m_abilityState = AbilityState::None;
	ConstructionState m_constructionState = ConstructionState::None;
	CombatState m_combatState = CombatState::None;

	void InstantiateComponentForEntity(ArgusEntity& entity) const override;
	bool MatchesType(UComponentData* other) const override;
};

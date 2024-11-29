// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class UAbilityRecord;

class AbilitySystems
{
public:
	static void RunSystems(float deltaTime);

	struct AbilitySystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		const AbilityComponent* m_abilityComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};

	static void CastAbility(uint32 abilityRecordId, const AbilitySystemsComponentArgs& components);

private:
	static void ProcessAbilityTaskCommands(const AbilitySystemsComponentArgs& components);
	static void CastSpawnAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components);
	static void CastHealAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components);
	static void CastAttackAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsComponentArgs& components);
	static void LogAbilityRecordError(const WIDECHAR* functionName);
};
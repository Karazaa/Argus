// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/AbilitySystemsArgs.h"

class UAbilityRecord;
struct ReticleComponent;

class AbilitySystems
{
public:
	static void RunSystems(float deltaTime);

	static void CastAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);
	static void PrepReticle(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);

private:
	static void ProcessAbilityTaskCommands(const AbilitySystemsArgs& components);
	static bool CastSpawnAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components, bool needsConstruction, bool atReticle);
	static bool CastHealAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);
	static bool CastAttackAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);
	static bool CastVacateAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);
	static void PrepReticleForConstructAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);
	static void LogAbilityRecordError(const WIDECHAR* functionName);
};
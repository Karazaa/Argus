// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/AbilitySystemsArgs.h"

class UAbilityRecord;
struct FAbilityEffect;
struct ReticleComponent;

class AbilitySystems
{
public:
	static void RunSystems(float deltaTime);

	static void CastAbility(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);
	static void PrepReticle(const UAbilityRecord* abilityRecord, const AbilitySystemsArgs& components);

private:
	static void ProcessAbilityTaskCommands(const AbilitySystemsArgs& components);
	static bool CastSpawnAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components, bool needsConstruction, bool atReticle);
	static bool CastHealAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components);
	static bool CastAttackAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components);
	static bool CastVacateAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components);
	static bool CastFlightTransitionAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components, bool landing);
	static bool CastAbilityOverrideAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components, bool adding);
	static void PrepReticleForConstructAbility(const UAbilityRecord* abilityRecord, const FAbilityEffect& abilityEffect, const AbilitySystemsArgs& components);
	static void LogAbilityRecordError(const WIDECHAR* functionName);
};
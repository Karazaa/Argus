// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/CombatSystemsArgs.h"

class CombatSystems
{
public:
	static void RunSystems(float deltaTime);
	static bool CanEntityAttackOtherEntity(ArgusEntity potentialAttacker, ArgusEntity potentialVictim);

private:
	static void ProcessCombatTaskCommands(float deltaTime, const CombatSystemsArgs& components);
	static void ProcessAttackCommand(float deltaTime, const CombatSystemsArgs& components);
	static void ProcessAttackMoveCommand(float deltaTime, const CombatSystemsArgs& components);
	static void PerformTimerAttack(ArgusEntity targetEntity, const CombatSystemsArgs& components);
	static void PerformContinuousAttack(float deltaTime, ArgusEntity targetEntity, const CombatSystemsArgs& components);
	static void ApplyDamage(uint32 damageAmount, ArgusEntity targetEntity, const CombatSystemsArgs& components);
	static void KillEntity(ArgusEntity targetEntity, HealthComponent* targetHealthComponent);
	static void StopAttackingEntity(const CombatSystemsArgs& components);
	static bool CanAttackGrounded(const CombatSystemsArgs& components);
	static bool CanAttackFlying(const CombatSystemsArgs& components);
};
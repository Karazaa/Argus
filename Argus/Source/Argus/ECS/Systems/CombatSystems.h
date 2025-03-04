// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class CombatSystems
{
public:
	static void RunSystems(float deltaTime);

	struct CombatSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		CombatComponent* m_combatComponent = nullptr;
		IdentityComponent* m_identityComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;
		TaskComponent* m_taskComponent = nullptr;
		TransformComponent* m_transformComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};

private:
	static void ProcessCombatTaskCommands(float deltaTime, const CombatSystemsComponentArgs& components);
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"

class SpawningSystems
{
public:
	static void RunSystems(float deltaTime);

	struct SpawningSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		TaskComponent* m_taskComponent = nullptr;
		SpawningComponent* m_spawningComponent = nullptr;

		bool AreComponentsValidCheck(bool shouldErrorIfInvalid, const WIDECHAR* functionName = nullptr) const;
	};

	static void SpawnEntity(const SpawningSystemsComponentArgs& components, const UArgusActorRecord* argusActorRecord);

private:
	static void ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsComponentArgs& components);
};
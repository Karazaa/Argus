// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntity.h"
#include "Misc/Optional.h"

class SpawningSystems
{
public:
	static void RunSystems(float deltaTime);

	struct SpawningSystemsComponentArgs
	{
		ArgusEntity m_entity = ArgusEntity::k_emptyEntity;
		SpawningComponent* m_spawningComponent = nullptr;
		TaskComponent* m_taskComponent = nullptr;
		TargetingComponent* m_targetingComponent = nullptr;
		TransformComponent* m_transformComponent = nullptr;

		bool AreComponentsValidCheck(const WIDECHAR* functionName) const;
	};

	static void SpawnEntity(const SpawningSystemsComponentArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord = nullptr);

private:
	static void SpawnEntityInternal(const SpawningSystemsComponentArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord);
	static void SpawnEntityFromQueue(const SpawningSystemsComponentArgs& components);
	static void ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsComponentArgs& components);
	static void ProcessQueuedSpawnEntity(const SpawningSystemsComponentArgs& components);
	static void GetSpawnLocationAndNavigationState(const SpawningSystemsComponentArgs& components, FVector& outSpawnLocation, MovementState& outMovementState);
};
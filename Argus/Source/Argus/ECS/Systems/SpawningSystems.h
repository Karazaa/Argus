// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "SystemArgumentDefinitions/SpawningSystemsArgs.h"

class SpawningSystems
{
public:
	static bool RunSystems(float deltaTime);

	static void SpawnEntity(const SpawningSystemsArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord = nullptr);

private:
	static void SpawnEntityInternal(const SpawningSystemsArgs& components, const SpawnEntityInfo& spawnInfo, const UArgusActorRecord* overrideArgusActorRecord);
	static void SpawnEntityFromQueue(const SpawningSystemsArgs& components);
	static bool ProcessSpawningTaskCommands(float deltaTime, const SpawningSystemsArgs& components);
	static bool ProcessQueuedSpawnEntity(const SpawningSystemsArgs& components);
	static void GetSpawnLocationAndNavigationState(const SpawningSystemsArgs& components, FVector& outSpawnLocation, EMovementState& outMovementState);
	static void CommandMoveSelectedEntitiesToSpawnedEntity(const ArgusEntity& spawnedEntity, bool requireConstructionTarget = false);
};
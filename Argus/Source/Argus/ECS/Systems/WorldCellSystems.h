// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Engine/World.h"

class UWorldCellRecord;
struct SpatialPartitioningComponent;
struct WorldCellComponent;
struct WorldCellSystemsArgs;
struct WorldReferenceComponent;

class WorldCellSystems
{
public:
	static void RunSystems(float deltaTime);

	static const UWorldCellRecord* GetWorldCellRecordForIndicies(TSoftObjectPtr<UWorld>& persistentWorld, int32 worldCellX, int32 worldCellY);
	static const UWorldCellRecord* GetWorldCellRecordPerEntity(const WorldCellComponent* worldCellComponent, const WorldReferenceComponent* worldReferenceComponent);

private:
	static void UpdateWorldCellLocationPerEntity(const WorldCellSystemsArgs& components, const SpatialPartitioningComponent* spatialPartitioningComponent);
};

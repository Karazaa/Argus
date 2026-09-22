// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "WorldCellSystems.h"
#include "ArgusIterators.h"
#include "ArgusLogging.h"
#include "ArgusMacros.h"
#include "ArgusMath.h"
#include "ArgusStaticData.h"
#include "RecordDefinitions/PersistentWorldTranslationRecord.h"
#include "RecordDefinitions/WorldCellIndexTranslationRecord.h"
#include "RecordDefinitions/WorldCellRecord.h"
#include "SystemArgumentDefinitions/WorldCellSystemsArgs.h"

void WorldCellSystems::RunSystems(float deltaTime)
{
	ARGUS_TRACE(WorldCellSystems::RunSystems);

	const SpatialPartitioningComponent* spatialPartitioningComponent = ArgusEntity::GetSingletonEntity().GetComponent<SpatialPartitioningComponent>();
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);
	const WorldReferenceComponent* worldReferenceComponent = ArgusEntity::GetSingletonEntity().GetComponent<WorldReferenceComponent>();
	ARGUS_RETURN_ON_NULL(worldReferenceComponent, ArgusECSLog);

	ArgusIterators::IterateSystemsArgs<WorldCellSystemsArgs>([spatialPartitioningComponent, worldReferenceComponent](const WorldCellSystemsArgs& components)
	{
		UpdateWorldCellLocationPerEntity(components, spatialPartitioningComponent);
		// GetWorldCellRecordPerEntity(components.m_worldCellComponent, worldReferenceComponent);
	});
}

const UWorldCellRecord* WorldCellSystems::GetWorldCellRecordForIndicies(TSoftObjectPtr<UWorld>& persistentWorld, int32 worldCellX, int32 worldCellY)
{
	ARGUS_TRACE(WorldCellSystems::GetWorldCellRecordForIndicies);

	if (persistentWorld.IsNull())
	{
		ARGUS_ERROR_NULL(ArgusECSLog, persistentWorld);
		return nullptr;
	}

	const UPersistentWorldTranslationRecord* persistentWorldTranslationRecord = ArgusStaticData::GetRecord<UPersistentWorldTranslationRecord>(ArgusECSConstants::k_persistentWorldTranslationRecordId);
	ARGUS_RETURN_ON_NULL_POINTER(persistentWorldTranslationRecord, ArgusECSLog);

	const uint32* worldCellIndexTranslationRecordId = persistentWorldTranslationRecord->m_persistentWorldToRecordId.Find(persistentWorld);
	ARGUS_RETURN_ON_NULL_POINTER(worldCellIndexTranslationRecordId, ArgusECSLog);

	const UWorldCellIndexTranslationRecord* worldCellIndexTranslationRecord = ArgusStaticData::GetRecord<UWorldCellIndexTranslationRecord>(*worldCellIndexTranslationRecordId);
	ARGUS_RETURN_ON_NULL_POINTER(worldCellIndexTranslationRecord, ArgusECSLog);

	const FCellIndexKey cellIndexKey = FCellIndexKey(worldCellX, worldCellY);
	const uint32* worldCellRecordId = worldCellIndexTranslationRecord->m_cellIndexRecordMapping.Find(cellIndexKey);
	ARGUS_RETURN_ON_NULL_POINTER(worldCellRecordId, ArgusECSLog);

	return ArgusStaticData::GetRecord<UWorldCellRecord>(*worldCellRecordId);
}

const UWorldCellRecord* WorldCellSystems::GetWorldCellRecordPerEntity(const WorldCellComponent* worldCellComponent, const WorldReferenceComponent* worldReferenceComponent)
{
	ARGUS_TRACE(WorldCellSystems::GetWorldCellRecordPerEntity);

	ARGUS_RETURN_ON_NULL_POINTER(worldCellComponent, ArgusECSLog);
	ARGUS_RETURN_ON_NULL_POINTER(worldReferenceComponent, ArgusECSLog);

	TSoftObjectPtr<UWorld> worldSoftPointer = TSoftObjectPtr<UWorld>(worldReferenceComponent->m_persistentWorldSoftObjectPath);
	const UWorldCellRecord* worldCellRecord = GetWorldCellRecordForIndicies(worldSoftPointer, worldCellComponent->m_cellXCoordinate, worldCellComponent->m_cellYCoordinate);
	ARGUS_RETURN_ON_NULL_POINTER(worldCellRecord, ArgusECSLog);

	return worldCellRecord;
}

void WorldCellSystems::UpdateWorldCellLocationPerEntity(const WorldCellSystemsArgs& components, const SpatialPartitioningComponent* spatialPartitioningComponent)
{
	if (!components.AreComponentsValidCheck(ARGUS_FUNCNAME))
	{
		return;
	}
	ARGUS_RETURN_ON_NULL(spatialPartitioningComponent, ArgusECSLog);

	const float cellX = ArgusMath::SafeDivide(components.m_transformComponent->m_location.Y, spatialPartitioningComponent->m_worldCellExtent);
	const float cellY = ArgusMath::SafeDivide(components.m_transformComponent->m_location.X, spatialPartitioningComponent->m_worldCellExtent);
	
	components.m_worldCellComponent->m_cellXCoordinate = cellX < 0.0f ? FMath::CeilToInt32(cellX) : FMath::FloorToInt32(cellX);
	components.m_worldCellComponent->m_cellYCoordinate = cellY < 0.0f ? FMath::CeilToInt32(cellY) : FMath::FloorToInt32(cellY);
}

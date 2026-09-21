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

	ArgusIterators::IterateSystemsArgs<WorldCellSystemsArgs>([spatialPartitioningComponent](const WorldCellSystemsArgs& components)
	{
		UpdateWorldCellLocationPerEntity(components, spatialPartitioningComponent);
	});
}

const UWorldCellRecord* WorldCellSystems::GetWorldCellRecordForIndicies(TSoftObjectPtr<UWorld>& persistentWorld, int32 worldCellX, int32 worldCellY)
{
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

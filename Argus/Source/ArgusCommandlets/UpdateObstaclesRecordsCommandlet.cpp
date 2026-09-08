// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdateObstaclesRecordsCommandlet.h"
#include "ArgusECSCommandletInterface.h"
#include "ArgusStaticData.h"
#include "Engine/AssetManager.h"
#include "RecordDefinitions/WorldCellRecord.h"
#include "RecordDefinitions/ObstaclesRecord.h"
#include "Systems/SpatialPartitioningSystems.h"

void UUpdateObstaclesRecordsCommandlet::OnStart()
{
	TSharedPtr<FStreamableHandle> loadDatabaseHandle = UAssetManager::Get().LoadPrimaryAssetsWithType(FPrimaryAssetType(UArgusStaticDatabase::StaticClass()->GetFName()));
	if (loadDatabaseHandle.IsValid())
	{
		loadDatabaseHandle->WaitUntilComplete();
	}

	ArgusECSCommandletInterface::InitializeECSForCommandlet();
}

int32 UUpdateObstaclesRecordsCommandlet::DoWork()
{
	ArgusStaticData::IterateAllRecordsOfType<UWorldCellRecord>([this](UWorldCellRecord* worldCellRecord)
	{
		if (!worldCellRecord)
		{
			return;
		}

		UObstaclesRecord* obstaclesRecord = const_cast<UObstaclesRecord*>(ArgusStaticData::GetRecord<UObstaclesRecord>(worldCellRecord->m_obstaclesRecord.GetId()));
		if (obstaclesRecord)
		{
			// TODO JAMES: Need way of loading level as a UWorld for the Commandlet.
			// Map soft reference is stored in WorldCellRecord.
			// 
			// 1) Load package
			// 2) UWorld::FindWorldInPackage
			// 3) World->WorldType = EWorldType::Editor;
			// 4) World->AddToRoot
			// 5) Initialize World
			// 
			// SpatialPartitioningSystems::GatherAvoidanceObstacles(nullptr, 8000.0f, obstaclesRecord->m_obstaclesContainer);

			obstaclesRecord->Modify(true);
			SaveDataAsset(obstaclesRecord);
		}
	});

	return 0;
}

void UUpdateObstaclesRecordsCommandlet::OnFinish()
{
	ArgusECSCommandletInterface::TeardownECSForCommandlet();
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdateObstaclesRecordsCommandlet.h"
#include "ArgusECSCommandletInterface.h"
#include "ArgusStaticData.h"
#include "Engine/AssetManager.h"
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
	ArgusStaticData::IterateAllRecordsOfType<UObstaclesRecord>([this](UObstaclesRecord* obstaclesRecord)
		{
			if (obstaclesRecord)
			{
				// TODO JAMES: Need way of loading level as a UWorld for the Commandlet.
				// Look at UEditorLoadingAndSavingUtils::LoadMap for guidance. 
				// We might want to include soft object references to levels in the obstacle record itself. 
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
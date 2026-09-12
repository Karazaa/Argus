// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdateObstaclesRecordsCommandlet.h"
#include "ArgusECSCommandletInterface.h"
#include "ArgusStaticData.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
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
		if (!worldCellRecord || worldCellRecord->m_worldReference.IsNull())
		{
			return;
		}

		UObstaclesRecord* obstaclesRecord = const_cast<UObstaclesRecord*>(ArgusStaticData::GetRecord<UObstaclesRecord>(worldCellRecord->m_obstaclesRecord.GetId()));
		if (!obstaclesRecord)
		{
			return;
		}

		const FString packageName = worldCellRecord->m_worldReference.GetLongPackageName();
		if (UWorld* loadedWorld = LoadWorld(packageName))
		{
			UE_LOG(LogTemp, Display, TEXT("===> Successfully loaded world: %s"), *loadedWorld->GetName());

			if (UNavigationSystemV1* unrealNavigationSystem = UNavigationSystemV1::GetCurrent(loadedWorld))
			{
				unrealNavigationSystem->InitializeForWorld(*loadedWorld, FNavigationSystemRunMode::EditorMode);
				SpatialPartitioningSystems::GatherAvoidanceObstacles(loadedWorld, FVector::ZeroVector, 8000.0f, obstaclesRecord->m_obstaclesContainer);
			}
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("===> Failed to load world at path: %s"), *packageName);
		}

		obstaclesRecord->Modify(true);
		SaveDataAsset(obstaclesRecord);
	});

	return 0;
}

void UUpdateObstaclesRecordsCommandlet::OnFinish()
{
	ArgusECSCommandletInterface::TeardownECSForCommandlet();
}
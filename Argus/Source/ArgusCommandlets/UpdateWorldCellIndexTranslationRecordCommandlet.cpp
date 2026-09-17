// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdateWorldCellIndexTranslationRecordCommandlet.h"
#include "ArgusECSCommandletInterface.h"
#include "ArgusStaticData.h"
#include "Engine/AssetManager.h"
#include "RecordDefinitions/WorldCellRecord.h"
#include "RecordDefinitions/WorldCellIndexTranslationRecord.h"

void UUpdateWorldCellIndexTranslationRecordCommandlet::OnStart()
{
	TSharedPtr<FStreamableHandle> loadDatabaseHandle = UAssetManager::Get().LoadPrimaryAssetsWithType(FPrimaryAssetType(UArgusStaticDatabase::StaticClass()->GetFName()));
	if (loadDatabaseHandle.IsValid())
	{
		loadDatabaseHandle->WaitUntilComplete();
	}

	ArgusECSCommandletInterface::InitializeECSForCommandlet();
}

int32 UUpdateWorldCellIndexTranslationRecordCommandlet::DoWork()
{
	ArgusStaticData::IterateAllRecordsOfType<UWorldCellIndexTranslationRecord>([this](UWorldCellIndexTranslationRecord* worldCellTranslationRecord)
	{
		if (!worldCellTranslationRecord || worldCellTranslationRecord->m_worldReference.IsNull())
		{
			return;
		}

		ArgusStaticData::IterateAllRecordsOfType<UWorldCellRecord>([worldCellTranslationRecord](UWorldCellRecord* worldCellRecord)
		{
			if (!worldCellRecord || !worldCellTranslationRecord || worldCellRecord->m_worldReference.IsNull() || worldCellTranslationRecord->m_worldReference.IsNull())
			{
				return;
			}

			if (worldCellRecord->m_worldReference != worldCellTranslationRecord->m_worldReference)
			{
				return;
			}

			worldCellTranslationRecord->m_cellIndexRecordMapping[FCellIndexKey(worldCellRecord->m_cellXCoordinate, worldCellRecord->m_cellYCoordinate)] = worldCellRecord->m_id;
		});

		worldCellTranslationRecord->Modify(true);
		SaveDataAsset(worldCellTranslationRecord);
	});

	return 0u;
}

void UUpdateWorldCellIndexTranslationRecordCommandlet::OnFinish()
{
	ArgusECSCommandletInterface::TeardownECSForCommandlet();
}
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdatePersistentWorldTranslationRecordCommandlet.h"
#include "ArgusECSCommandletInterface.h"
#include "ArgusStaticData.h"
#include "Engine/AssetManager.h"
#include "RecordDefinitions/PersistentWorldTranslationRecord.h"
#include "RecordDefinitions/WorldCellIndexTranslationRecord.h"

void UUpdatePersistentWorldTranslationRecordCommandlet::OnStart()
{
	TSharedPtr<FStreamableHandle> loadDatabaseHandle = UAssetManager::Get().LoadPrimaryAssetsWithType(FPrimaryAssetType(UArgusStaticDatabase::StaticClass()->GetFName()));
	if (loadDatabaseHandle.IsValid())
	{
		loadDatabaseHandle->WaitUntilComplete();
	}

	ArgusECSCommandletInterface::InitializeECSForCommandlet();
}

int32 UUpdatePersistentWorldTranslationRecordCommandlet::DoWork()
{
	ArgusStaticData::IterateAllRecordsOfType<UPersistentWorldTranslationRecord>([this](UPersistentWorldTranslationRecord* persistentWorldTranslationRecord)
	{
		if (!persistentWorldTranslationRecord)
		{
			return;
		}

		ArgusStaticData::IterateAllRecordsOfType<UWorldCellIndexTranslationRecord>([persistentWorldTranslationRecord](UWorldCellIndexTranslationRecord* worldCellIndexTranslationRecord)
		{
			if (!worldCellIndexTranslationRecord)
			{
				return;
			}

			if (uint32* foundRecordId = persistentWorldTranslationRecord->m_persistentWorldToRecordId.Find(worldCellIndexTranslationRecord->m_worldReference))
			{
				*foundRecordId = worldCellIndexTranslationRecord->m_id;
			}
			else
			{
				persistentWorldTranslationRecord->m_persistentWorldToRecordId.Add(worldCellIndexTranslationRecord->m_worldReference, worldCellIndexTranslationRecord->m_id);
			}
		});

		persistentWorldTranslationRecord->Modify(true);
		SaveDataAsset(persistentWorldTranslationRecord);
	});

	return 0;
}

void UUpdatePersistentWorldTranslationRecordCommandlet::OnFinish()
{
	ArgusECSCommandletInterface::TeardownECSForCommandlet();
}

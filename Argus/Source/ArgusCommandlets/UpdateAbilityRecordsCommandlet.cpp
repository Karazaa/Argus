// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdateAbilityRecordsCommandlet.h"
#include "ArgusECSCommandletInterface.h"
#include "ArgusStaticData.h"
#include "Engine/AssetManager.h"

void UUpdateAbilityRecordsCommandlet::OnStart()
{
	TSharedPtr<FStreamableHandle> loadDatabaseHandle = UAssetManager::Get().LoadPrimaryAssetsWithType(FPrimaryAssetType(UArgusStaticDatabase::StaticClass()->GetFName()));
	if (loadDatabaseHandle.IsValid())
	{
		loadDatabaseHandle->WaitUntilComplete();
	}

	ArgusECSCommandletInterface::InitializeECSForCommandlet();
}

int32 UUpdateAbilityRecordsCommandlet::DoWork()
{
	ArgusStaticData::IterateAllRecordsOfType<UAbilityRecord>([this](UAbilityRecord* abilityRecord) 
	{
		if (abilityRecord)
		{
			abilityRecord->UpdateEntityCategoriesSpawnedByAbility();
			abilityRecord->Modify(true);
			SaveDataAsset(abilityRecord);
		}
	});

	return 0;
}

void UUpdateAbilityRecordsCommandlet::OnFinish()
{
	ArgusECSCommandletInterface::TeardownECSForCommandlet();
}
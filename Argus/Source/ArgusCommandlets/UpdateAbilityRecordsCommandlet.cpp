// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdateAbilityRecordsCommandlet.h"
#include "ArgusEntity.h"
#include "ArgusStaticData.h"

void UUpdateAbilityRecordsCommandlet::OnStart()
{
	// TODO JAMES: Need to support DLL export on more things in order to not run into linker errors.
	//ArgusEntity::FlushAllEntities();
	//ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusECSConstants::k_singletonEntityId);
	//singletonEntity.AddComponent<AssetLoadingComponent>();
}

int32 UUpdateAbilityRecordsCommandlet::DoWork()
{
	ArgusStaticData::IterateAllRecordsOfType<UAbilityRecord>([](UAbilityRecord* abilityRecord) 
	{
		if (abilityRecord)
		{
			abilityRecord->UpdateEntityCategoriesSpawnedByAbility();
		}
	});

	return 0;
}

void UUpdateAbilityRecordsCommandlet::OnFinish()
{
	//ArgusEntity::FlushAllEntities();
}
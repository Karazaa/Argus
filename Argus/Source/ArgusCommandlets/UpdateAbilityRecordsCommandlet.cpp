// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "UpdateAbilityRecordsCommandlet.h"
#include "ArgusECSCommandletInterface.h"
#include "ArgusStaticData.h"

void UUpdateAbilityRecordsCommandlet::OnStart()
{
	ArgusECSCommandletInterface::InitializeECSForCommandlet();
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
	ArgusECSCommandletInterface::TeardownECSForCommandlet();
}
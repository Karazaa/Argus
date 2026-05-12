// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "PlacedArgusActorTeamInfoRecordFactory.h"
#include "ArgusEditorModule.h"

UPlacedArgusActorTeamInfoRecordFactory::UPlacedArgusActorTeamInfoRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UPlacedArgusActorTeamInfoRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UPlacedArgusActorTeamInfoRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_PlacedArgusActorTeamInfoRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

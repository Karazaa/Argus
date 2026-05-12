// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamColorRecordFactory.h"
#include "ArgusEditorModule.h"

UTeamColorRecordFactory::UTeamColorRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UTeamColorRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UTeamColorRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_TeamColorRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

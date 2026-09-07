// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "WorldCellRecordFactory.h"
#include "ArgusEditorModule.h"

UWorldCellRecordFactory::UWorldCellRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UWorldCellRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UWorldCellRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_WorldCellRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

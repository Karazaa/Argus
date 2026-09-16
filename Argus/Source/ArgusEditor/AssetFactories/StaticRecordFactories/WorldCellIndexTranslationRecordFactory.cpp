// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "WorldCellIndexTranslationRecordFactory.h"
#include "ArgusEditorModule.h"

UWorldCellIndexTranslationRecordFactory::UWorldCellIndexTranslationRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UWorldCellIndexTranslationRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UWorldCellIndexTranslationRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_WorldCellIndexTranslationRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

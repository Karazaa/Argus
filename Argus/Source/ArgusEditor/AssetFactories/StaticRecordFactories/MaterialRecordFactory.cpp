// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "MaterialRecordFactory.h"
#include "ArgusEditorModule.h"

UMaterialRecordFactory::UMaterialRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UMaterialRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UMaterialRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_MaterialRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

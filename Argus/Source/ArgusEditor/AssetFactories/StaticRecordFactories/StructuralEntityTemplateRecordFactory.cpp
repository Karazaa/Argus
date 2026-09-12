// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "StructuralEntityTemplateRecordFactory.h"
#include "ArgusEditorModule.h"

UStructuralEntityTemplateRecordFactory::UStructuralEntityTemplateRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UStructuralEntityTemplateRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UStructuralEntityTemplateRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_StructuralEntityTemplateRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

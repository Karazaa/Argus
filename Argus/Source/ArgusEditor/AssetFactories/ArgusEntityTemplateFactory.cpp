// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEntityTemplateFactory.h"
#include "ArgusEditorModule.h"

UArgusEntityTemplateFactory::UArgusEntityTemplateFactory(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	SupportedClass = UArgusEntityTemplate::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UArgusEntityTemplateFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_ArgusEntityTemplate::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ResourceSetRecordFactory.h"
#include "ArgusEditorModule.h"

UResourceSetRecordFactory::UResourceSetRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UResourceSetRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UResourceSetRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_ResourceSetRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "FactionRecordFactory.h"
#include "ArgusEditorModule.h"

UFactionRecordFactory::UFactionRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UFactionRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UFactionRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_FactionRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

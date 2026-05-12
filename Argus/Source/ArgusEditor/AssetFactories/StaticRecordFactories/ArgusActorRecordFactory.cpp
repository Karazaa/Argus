// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorRecordFactory.h"
#include "ArgusEditorModule.h"

UArgusActorRecordFactory::UArgusActorRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UArgusActorRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UArgusActorRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_ArgusActorRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

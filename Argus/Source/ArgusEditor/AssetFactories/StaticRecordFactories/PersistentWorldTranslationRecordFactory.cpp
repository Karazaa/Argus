// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "PersistentWorldTranslationRecordFactory.h"
#include "ArgusEditorModule.h"

UPersistentWorldTranslationRecordFactory::UPersistentWorldTranslationRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UPersistentWorldTranslationRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UPersistentWorldTranslationRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_PersistentWorldTranslationRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

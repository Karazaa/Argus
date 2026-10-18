// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "HeadlessEntityRecordFactory.h"
#include "ArgusEditorModule.h"

UHeadlessEntityRecordFactory::UHeadlessEntityRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UHeadlessEntityRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UHeadlessEntityRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_HeadlessEntityRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

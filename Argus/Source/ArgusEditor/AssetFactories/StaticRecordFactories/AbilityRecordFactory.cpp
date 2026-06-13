// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "AbilityRecordFactory.h"
#include "ArgusEditorModule.h"

UAbilityRecordFactory::UAbilityRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UAbilityRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAbilityRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_AbilityRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

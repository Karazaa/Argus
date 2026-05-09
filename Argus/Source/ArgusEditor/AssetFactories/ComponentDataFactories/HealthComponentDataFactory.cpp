// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "HealthComponentDataFactory.h"
#include "ArgusEditorModule.h"

UHealthComponentDataFactory::UHealthComponentDataFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UHealthComponentData::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UHealthComponentDataFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_HealthComponentData::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ObstaclesRecordFactory.h"
#include "ArgusEditorModule.h"

UObstaclesRecordFactory::UObstaclesRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UObstaclesRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UObstaclesRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_ObstaclesRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

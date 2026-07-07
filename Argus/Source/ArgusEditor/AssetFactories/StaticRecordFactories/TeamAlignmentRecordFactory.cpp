// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "TeamAlignmentRecordFactory.h"
#include "ArgusEditorModule.h"

UTeamAlignmentRecordFactory::UTeamAlignmentRecordFactory(const FObjectInitializer& objectInitializer)
{
	SupportedClass = UTeamAlignmentRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UTeamAlignmentRecordFactory::FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning)
{
	if (!ensure(classDefinition) || !ensure(classDefinition->IsChildOf(SupportedClass)))
	{
		return nullptr;
	}

	return NewObject<UObject>(parentObject, SupportedClass, name, objectFlags | RF_Transactional, context);
}

uint32 FAssetTypeActions_TeamAlignmentRecord::GetCategories()
{
	return ArgusEditorModule::GetAssetTypeCategory();
}

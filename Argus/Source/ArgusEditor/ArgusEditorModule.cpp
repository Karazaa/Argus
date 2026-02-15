// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusEditorModule.h"
#include "Argus/StaticData/ArgusStaticRecordReference.h"
#include "UObject/UObjectGlobals.h"

#define LOCTEXT_NAMESPACE "ArgusEditor"

IMPLEMENT_MODULE(ArgusEditorModule, ArgusEditor);

namespace
{
	template<typename StructType>
	const StructType* CastStructValue(const FStructProperty* structProperty, const void* value)
	{
		check(structProperty && value);

		if (structProperty->Struct->IsChildOf(TBaseStructure<StructType>::Get()))
		{
			return reinterpret_cast<const StructType*>(value);
		}

		return nullptr;
	}
}

ArgusEditorModule& ArgusEditorModule::Get()
{
	return FModuleManager::GetModuleChecked<ArgusEditorModule>("ArgusEditor");
}

void ArgusEditorModule::StartupModule()
{
	m_propertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &ArgusEditorModule::OnObjectPropertyChanged);
}

void ArgusEditorModule::ShutdownModule()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(m_propertyChangedHandle);
}

void ArgusEditorModule::OnObjectPropertyChanged(UObject* object, FPropertyChangedEvent& propertyChangedEvent)
{
	if (!object || !propertyChangedEvent.MemberProperty)
	{
		return;
	}

	const FStructProperty* structProperty = CastField<FStructProperty>(propertyChangedEvent.MemberProperty);
	if (!structProperty)
	{
		return;
	}

	const void* value = structProperty->ContainerPtrToValuePtr<void>(object);
	const UStruct* structDefinition = structProperty->Struct;
	if (!value || !structDefinition)
	{
		return;
	}

	if (const FArgusStaticRecordReference* recordInstance = CastStructValue<FArgusStaticRecordReference>(structProperty, value))
	{
		recordInstance->StoreId();
	}
}

#undef LOCTEXT_NAMESPACE
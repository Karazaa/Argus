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
		check(structProperty && structProperty->Struct && value);

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

void ArgusEditorModule::OnObjectPropertyChanged(UObject* object, FPropertyChangedEvent& propertyChangedEvent) const
{
	const FProperty* initialProperty = propertyChangedEvent.MemberProperty;
	if (!object || !initialProperty || !propertyChangedEvent.Property)
	{
		return;
	}

	if (!CastField<FStructProperty>(initialProperty) && !CastField<FArrayProperty>(initialProperty))
	{
		return;
	}

	StoreRecordReferenceRecursive(initialProperty->ContainerPtrToValuePtr<void>(object), initialProperty, propertyChangedEvent.Property);
}

void ArgusEditorModule::StoreRecordReferenceRecursive(const void* container, const FProperty* currentProperty, const FProperty* targetProperty) const
{
	if (!container || !currentProperty || !targetProperty)
	{
		return;
	}

	if (const FArrayProperty* arrayProperty = CastField<FArrayProperty>(currentProperty))
	{
		FScriptArrayHelper arrayHelper = FScriptArrayHelper(arrayProperty, container);
		for (int32 i = 0; i < arrayHelper.Num(); ++i)
		{
			StoreRecordReferenceRecursive(arrayHelper.GetRawPtr(i), arrayProperty->Inner, targetProperty);
		}
		return;
	}

	const FStructProperty* structProperty = CastField<FStructProperty>(currentProperty);
	if (!structProperty || !structProperty->Struct)
	{
		return;
	}

	const FField* childField = structProperty->Struct->ChildProperties;
	while (childField)
	{
		if (const FProperty* childProperty = CastField<FProperty>(childField))
		{
			if (childProperty == targetProperty)
			{
				if (const FArgusStaticRecordReference* recordReference = CastStructValue<FArgusStaticRecordReference>(structProperty, container))
				{
					recordReference->StoreId();
					return;
				}
			}

			const void* childValue = childProperty->ContainerPtrToValuePtr<void>(container);
			StoreRecordReferenceRecursive(childValue, childProperty, targetProperty);
		}

		childField = childField->Next;
	}
}

#undef LOCTEXT_NAMESPACE
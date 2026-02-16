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
	if (!object || !initialProperty)
	{
		return;
	}

	if (!CastField<FStructProperty>(initialProperty) && !CastField<FArrayProperty>(initialProperty))
	{
		return;
	}

	if (const FArgusStaticRecordReference* recordInstance = FindRecordReferenceRecursive(initialProperty->ContainerPtrToValuePtr<void>(object), initialProperty))
	{
		recordInstance->StoreId();
	}
}

const FArgusStaticRecordReference* ArgusEditorModule::FindRecordReferenceRecursive(const void* container, const FProperty* currentProperty) const
{
	if (!container || !currentProperty)
	{
		return nullptr;
	}

	if (const FArrayProperty* arrayProperty = CastField<FArrayProperty>(currentProperty))
	{
		FScriptArrayHelper arrayHelper = FScriptArrayHelper(arrayProperty, container);
		for (int32 i = 0; i < arrayHelper.Num(); ++i)
		{
			void* innerValue = arrayHelper.GetRawPtr(i);
			const FProperty* innerProperty = arrayProperty->Inner;
			if (const FArgusStaticRecordReference* recordInstance = FindRecordReferenceRecursive(innerValue, innerProperty))
			{
				return recordInstance;
			}
		}
	}

	if (const FStructProperty* structProperty = CastField<FStructProperty>(currentProperty))
	{
		if (const FArgusStaticRecordReference* recordInstance = CastStructValue<FArgusStaticRecordReference>(structProperty, container))
		{
			return recordInstance;
		}

		if(structProperty->Struct)
		{
			const FField* childField = structProperty->Struct->ChildProperties;
			while (childField)
			{
				if (const FProperty* childProperty = CastField<FProperty>(childField))
				{
					const void* childValue = childProperty->ContainerPtrToValuePtr<void>(container);
					if (const FArgusStaticRecordReference* recordInstance = FindRecordReferenceRecursive(childValue, childProperty))
					{
						return recordInstance;
					}
				}
				
				childField = childField->Next;
			}
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
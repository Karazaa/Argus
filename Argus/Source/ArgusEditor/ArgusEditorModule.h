// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IPropertyHandle;
struct FArgusStaticRecordReference;
struct FPropertyChangedEvent;

class ArgusEditorModule : public IModuleInterface
{
public:
	static ArgusEditorModule& Get();

	void StartupModule() override;
	void ShutdownModule() override;

	bool IsGameModule() const override { return false; }

private:
	void OnObjectPropertyChanged(UObject* object, FPropertyChangedEvent& propertyChangedEvent) const;
	const FArgusStaticRecordReference* FindRecordReferenceRecursive(const void* container, const FProperty* currentProperty) const;

	FDelegateHandle m_propertyChangedHandle;
};
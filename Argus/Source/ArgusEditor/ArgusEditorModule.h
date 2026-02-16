// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

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
	void StoreRecordReferenceRecursive(const void* container, const FProperty* currentProperty, const FProperty* targetProperty) const;

	FDelegateHandle m_propertyChangedHandle;
};
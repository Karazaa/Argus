// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "AssetTypeCategories.h"
#include "ArgusEditorGeneratedLogic.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FArgusStaticRecordReference;
struct FPropertyChangedEvent;

class ArgusEditorModule : public IModuleInterface
{
public:
	static ArgusEditorModule& Get();
	static EAssetTypeCategories::Type GetAssetTypeCategory();

	void StartupModule() override;
	void ShutdownModule() override;

	bool IsGameModule() const override { return false; }

private:
	static EAssetTypeCategories::Type s_argusAssetCategory;

	void OnObjectPropertyChanged(UObject* object, FPropertyChangedEvent& propertyChangedEvent) const;
	void StoreRecordReferenceRecursive(const void* container, const FProperty* currentProperty, const FProperty* targetProperty) const;
	void RegisterAssetActions(class IAssetTools& assetTools) const;

	FDelegateHandle m_propertyChangedHandle;

	EDITOR_MODULE_FRIENDS
};
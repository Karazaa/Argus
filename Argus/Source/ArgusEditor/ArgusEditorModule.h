// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IPropertyHandle;
struct FPropertyChangedEvent;

class ArgusEditorModule : public IModuleInterface
{
public:
	static ArgusEditorModule& Get();

	void StartupModule() override;
	void ShutdownModule() override;

	bool IsGameModule() const override { return false; }

private:
	void OnObjectPropertyChanged(UObject* object, FPropertyChangedEvent& propertyChangedEvent);

	FDelegateHandle m_propertyChangedHandle;
};
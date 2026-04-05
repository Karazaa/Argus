// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

class STextBlock;
class SEditableText;

class ArgusECSObjectAdder
{
public:
	enum class ECSType : uint8
	{
		Component,
		System,
		SystemArguments
	};

	enum class ComponentType : uint8
	{
		StaticAllocated,
		DynamicAllocated,
		Settings
	};

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	FReply OnClicked();
	void OnTextChanged(const FText& text);
	TSharedRef<SWidget> OnGetECSTypeContent();
	void OnECSTypeChange(int32 Index);
	TSharedRef<SWidget> OnGetComponentTypeContent();
	void OnComponentTypeChange(int32 Index);

private:
	FReply OnClicked_Component();
	FReply OnClicked_System();
	FReply OnClicked_SystemArgument();

	void ClearMessage();
	void MessageError(const FText& errorMessage);
	void MessageSuccess(const FText& successMessage);

	TSharedPtr<STextBlock> m_currentMessageText;
	TSharedPtr<STextBlock> m_ecsTypeDropDownText;
	TSharedPtr<STextBlock> m_componentTypeDropDownText;
	TSharedPtr<STextBlock> m_currentLabelText;
	TSharedPtr<SEditableText> m_currentHintText;
	TSharedPtr<SHorizontalBox> m_componentTypeBox;

	FText m_ecsTypeNameArray[3] = { FText::FromString(TEXT("Component")), FText::FromString(TEXT("System")), FText::FromString(TEXT("System Arguments")) };
	FText m_componentTypeNameArray[3] = { FText::FromString(TEXT("Static Allocated")), FText::FromString(TEXT("Dynamic Allocated")), FText::FromString(TEXT("Settings")) };
	FText m_inputFieldText;
	FText m_messageText;

	ECSType m_ecsType = ECSType::Component;
	ComponentType m_componentType = ComponentType::StaticAllocated;
};
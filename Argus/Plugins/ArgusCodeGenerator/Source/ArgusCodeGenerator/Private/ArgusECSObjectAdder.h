// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include <string>

class SDockTab;
class SEditableText;
class STextBlock;

struct ObjectAdderFileInfo
{
	const char* m_templateDirectorySuffix = nullptr;
	const char* m_outputDirectorySuffix = nullptr;
	const char* m_templateFileName;
};

class ArgusECSObjectAdder
{
public:
	enum class ECSType : uint8
	{
		Component,
		System,
		SystemArguments,
		StaticDataRecord,
		Commandlet,
		Count
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
	FReply OnClicked_StaticDataRecord();
	FReply OnClicked_Commandlet();

	void GenerateFiles(const std::string& inputString, ObjectAdderFileInfo* headerFileInfo, ObjectAdderFileInfo* implementationFileInfo = nullptr);
	void ClearMessage();
	void MessageError(const FText& errorMessage);
	void MessageSuccess(const FText& successMessage);

	TSharedPtr<STextBlock> m_currentMessageText;
	TSharedPtr<STextBlock> m_ecsTypeDropDownText;
	TSharedPtr<STextBlock> m_componentTypeDropDownText;
	TSharedPtr<STextBlock> m_currentLabelText;
	TSharedPtr<SEditableText> m_currentHintText;
	TSharedPtr<SHorizontalBox> m_componentTypeBox;

	FText m_ecsTypeNameArray[static_cast<uint8>(ECSType::Count)] = { FText::FromString(TEXT("Component")), FText::FromString(TEXT("System")), FText::FromString(TEXT("System Arguments")), FText::FromString(TEXT("Static Data Record")),  FText::FromString(TEXT("Commandlet")) };
	FText m_componentTypeNameArray[3] = { FText::FromString(TEXT("Static Allocated")), FText::FromString(TEXT("Dynamic Allocated")), FText::FromString(TEXT("Settings")) };
	FText m_inputFieldText;
	FText m_messageText;

	ECSType m_ecsType = ECSType::Component;
	ComponentType m_componentType = ComponentType::StaticAllocated;
};
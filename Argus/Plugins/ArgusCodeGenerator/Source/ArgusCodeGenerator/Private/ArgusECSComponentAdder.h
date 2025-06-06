// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

class ArgusECSComponentAdder
{
public:
	enum class ECSType : uint8
	{
		Component,
		System,
		SystemArguments
	};

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	FReply OnClicked();
	void OnCheckBoxChecked(ECheckBoxState checkBoxState);
	void OnTextChanged(const FText& text);
	TSharedRef<SWidget> OnGetECSTypeContent();
	void OnECSTypeChange(int32 Index);

private:
	TSharedPtr<STextBlock> m_currentDropDownText;
	FText m_nameArray[3] = { FText::FromString(TEXT("Component")), FText::FromString(TEXT("System")), FText::FromString(TEXT("System Arguments")) };
	FText m_inputFieldText;
	FText m_messageText;
	bool m_isDynamicallyAllocated;

	ECSType m_ecsType = ECSType::Component;
};
// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSComponentAdder.h"
#include "ArgusComponentRegistryCodeGenerator.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableText.h"

#define LOCTEXT_NAMESPACE "FArgusCodeGeneratorModule"

TSharedRef<SDockTab> ArgusECSComponentAdder::OnSpawnPluginTab(const FSpawnTabArgs& spawnTabArgs)
{
	m_inputFieldText = FText::FromString(TEXT(""));
	m_messageText = FText::FromString(TEXT(""));
	m_isDynamicallyAllocated = false;

	m_currentDropDownText = SNew(STextBlock).Text(m_nameArray[static_cast<uint8>(m_ecsType)]);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SBox).VAlign(VAlign_Top).HAlign(HAlign_Left)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.FillContentHeight(1.0f)
						.Padding(FMargin(10, 10, 10, 10))
						[
							SNew(SComboButton)
								.OnGetMenuContent(FOnGetContent::CreateRaw(this, &ArgusECSComponentAdder::OnGetECSTypeContent))
								.ContentPadding(FMargin(2.0f, 2.0f))
								.ButtonContent()
								[
									m_currentDropDownText.ToSharedRef()
								]
						]
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.FillContentHeight(1.0f)
						.Padding(FMargin(10, 10, 10, 10))
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								.HAlign(HAlign_Left)
								.FillContentWidth(1.0f)
								.Padding(FMargin(0, 0, 2, 0))
								[
									SNew(SBox).VAlign(VAlign_Top).HAlign(HAlign_Right)
										[
											SNew(STextBlock).Text(FText::FromString(TEXT("Component Name: ")))
										]
								]
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								.HAlign(HAlign_Left)
								.FillContentWidth(1.0f)
								.Padding(FMargin(2, 0, 0, 0))
								[
									SNew(SBox).VAlign(VAlign_Top).HAlign(HAlign_Left)
										[
											SNew(SEditableText).HintText(FText::FromString(TEXT("Enter component to add name here."))).ColorAndOpacity(FColor::Green)
												.OnTextChanged(FOnTextChanged::CreateRaw(this, &ArgusECSComponentAdder::OnTextChanged))
										]
								]
						]
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.FillContentHeight(1.0f)
						.Padding(FMargin(10, 10, 10, 10))
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								.HAlign(HAlign_Left)
								.FillContentWidth(1.0f)
								.Padding(FMargin(0, 0, 2, 0))
								[
									SNew(SBox).VAlign(VAlign_Top).HAlign(HAlign_Right)
										[
											SNew(STextBlock).Text(FText::FromString(TEXT("Is Dynamically Allocated? ")))
										]
								]
								+ SHorizontalBox::Slot()
								.VAlign(VAlign_Top)
								.HAlign(HAlign_Left)
								.FillContentWidth(1.0f)
								.Padding(FMargin(2, 0, 0, 0))
								[
									SNew(SBox).VAlign(VAlign_Top).HAlign(HAlign_Left)
										[
											SNew(SCheckBox).OnCheckStateChanged(FOnCheckStateChanged::CreateRaw(this, &ArgusECSComponentAdder::OnCheckBoxChecked))
										]
								]
						]
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						.FillContentHeight(1.0f)
						.Padding(FMargin(10, 10, 10, 10))
						[
							SNew(SBox).VAlign(VAlign_Top).HAlign(HAlign_Left)
								[
									SNew(SButton).Text(FText::FromString(TEXT("Add Component"))).OnClicked(FOnClicked::CreateRaw(this, &ArgusECSComponentAdder::OnClicked))
								]
						]
				]
		];
}

TSharedRef<SWidget> ArgusECSComponentAdder::OnGetECSTypeContent()
{
	FMenuBuilder MenuBuilder(true, NULL);
	for (uint8 i = 0u; i < 3; i++)
	{
		FUIAction itemAction = FUIAction(FExecuteAction::CreateRaw(this, &ArgusECSComponentAdder::OnECSTypeChange, static_cast<int32>(i)));
		MenuBuilder.AddMenuEntry(m_nameArray[i], TAttribute<FText>(), FSlateIcon(), itemAction);
	}

	return MenuBuilder.MakeWidget();
}

void ArgusECSComponentAdder::OnECSTypeChange(int32 Index)
{
	const uint8 enumValue = IntCastChecked<uint8>(Index);
	m_ecsType = static_cast<ECSType>(enumValue);
	m_currentDropDownText->SetText(m_nameArray[static_cast<uint8>(m_ecsType)]);
}

void ArgusECSComponentAdder::OnTextChanged(const FText& text)
{
	m_inputFieldText = text;
}

void ArgusECSComponentAdder::OnCheckBoxChecked(ECheckBoxState checkBoxState)
{
	m_isDynamicallyAllocated = checkBoxState == ECheckBoxState::Checked;
}

FReply ArgusECSComponentAdder::OnClicked()
{
	if (m_inputFieldText.IsEmpty())
	{
		m_messageText = FText::FromString(TEXT("No component name defined!"));
		return FReply::Handled();
	}

	// Parse from source files
	ArgusCodeGeneratorUtil::ParseComponentDataOutput parsedComponentData;
	ArgusCodeGeneratorUtil::ParseComponentData(parsedComponentData);

	std::string inputString = std::string(TCHAR_TO_UTF8(*m_inputFieldText.ToString()));
	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{
		if (parsedComponentData.m_componentNames[i].compare(inputString) == 0)
		{
			m_messageText = FText::FromString(TEXT("Component name already exists!"));
			return FReply::Handled();
		}
	}

	// Construct a directory path to component adder templates
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(ArgusComponentRegistryCodeGenerator::s_componentRegistryTemplateDirectorySuffix));

	// Parse per component template
	std::vector<std::string> parsedLines = std::vector<std::string>();
	std::vector<std::string> componentNames = std::vector<std::string>();
	componentNames.push_back(inputString);

	std::string templateFileName = m_isDynamicallyAllocated ? "DynamicAllocComponentAdderTemplate.txt" : "ComponentAdderTemplate.txt";
	ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(std::string(cStrTemplateDirectory).append(templateFileName), componentNames, parsedLines);

	// Construct a directory path to component definition location
	FString definitionsDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	std::string directorySuffix = m_isDynamicallyAllocated ? "Source/Argus/ECS/DynamicAllocComponentDefinitions/" : "Source/Argus/ECS/ComponentDefinitions/";
	definitionsDirectory.Append(directorySuffix.c_str());
	FPaths::MakeStandardFilename(definitionsDirectory);
	const char* cStrDefinitionsDirectory = ARGUS_FSTRING_TO_CHAR(definitionsDirectory);

	// Write out newly defined component
	ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrDefinitionsDirectory).append(inputString.append(".h")), parsedLines);
	return FReply::Handled();
}
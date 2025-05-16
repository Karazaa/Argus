// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSComponentAdder.h"
#include "ArgusComponentRegistryCodeGenerator.h"

#define LOCTEXT_NAMESPACE "FArgusCodeGeneratorModule"

FText ArgusECSComponentAdder::s_inputFieldText = FText::FromString(TEXT(""));
FText ArgusECSComponentAdder::s_messageText = FText::FromString(TEXT(""));
bool ArgusECSComponentAdder::s_isDynamicallyAllocated = false;

TSharedRef<SDockTab> ArgusECSComponentAdder::OnSpawnPluginTab(const FSpawnTabArgs& spawnTabArgs)
{
	s_inputFieldText = FText::FromString(TEXT(""));
	s_messageText = FText::FromString(TEXT(""));
	s_isDynamicallyAllocated = false;

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
											SNew(SEditableText).HintText(FText::FromString(TEXT("Enter component to add name here."))).ColorAndOpacity(FColor::Green).OnTextChanged(
												FOnTextChanged::CreateLambda([](const FText& text)
													{
														ArgusECSComponentAdder::OnTextChanged(text);
													})
											)
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
											SNew(SCheckBox).OnCheckStateChanged(FOnCheckStateChanged::CreateLambda([](ECheckBoxState checkBoxState)
												{
													ArgusECSComponentAdder::OnCheckBoxChecked(checkBoxState);
												}))
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
									SNew(SButton).Text(FText::FromString(TEXT("Add Component"))).OnClicked_Lambda([]()->FReply 
										{ 
											ArgusECSComponentAdder::OnClicked();
											return FReply::Handled(); 
										})
								]
						]
				]
		];
}

void ArgusECSComponentAdder::OnTextChanged(const FText& text)
{
	s_inputFieldText = text;
}

void ArgusECSComponentAdder::OnCheckBoxChecked(ECheckBoxState checkBoxState)
{
	s_isDynamicallyAllocated = checkBoxState == ECheckBoxState::Checked;
}

void ArgusECSComponentAdder::OnClicked()
{
	if (s_inputFieldText.IsEmpty())
	{
		s_messageText = FText::FromString(TEXT("No component name defined!"));
		return;
	}

	// Parse from source files
	ArgusCodeGeneratorUtil::ParseComponentDataOutput parsedComponentData;
	ArgusCodeGeneratorUtil::ParseComponentData(parsedComponentData);

	std::string inputString = std::string(TCHAR_TO_UTF8(*s_inputFieldText.ToString()));
	for (int i = 0; i < parsedComponentData.m_componentNames.size(); ++i)
	{
		if (parsedComponentData.m_componentNames[i].compare(inputString) == 0)
		{
			s_messageText = FText::FromString(TEXT("Component name already exists!"));
			return;
		}
	}

	// Construct a directory path to component adder templates
	const char* cStrTemplateDirectory = ARGUS_FSTRING_TO_CHAR(ArgusCodeGeneratorUtil::GetTemplateDirectory(ArgusComponentRegistryCodeGenerator::s_componentRegistryTemplateDirectorySuffix));

	// Parse per component template
	std::vector<std::string> parsedLines = std::vector<std::string>();
	std::vector<std::string> componentNames = std::vector<std::string>();
	componentNames.push_back(inputString);

	std::string templateFileName = s_isDynamicallyAllocated ? "DynamicAllocComponentAdderTemplate.txt" : "ComponentAdderTemplate.txt";
	ArgusCodeGeneratorUtil::ParseComponentSpecificTemplate(std::string(cStrTemplateDirectory).append(templateFileName), componentNames, parsedLines);

	// Construct a directory path to component definition location
	FString definitionsDirectory = ArgusCodeGeneratorUtil::GetProjectDirectory();
	std::string directorySuffix = s_isDynamicallyAllocated ? "Source/Argus/ECS/DynamicAllocComponentDefinitions/" : "Source/Argus/ECS/ComponentDefinitions/";
	definitionsDirectory.Append(directorySuffix.c_str());
	FPaths::MakeStandardFilename(definitionsDirectory);
	const char* cStrDefinitionsDirectory = ARGUS_FSTRING_TO_CHAR(definitionsDirectory);

	// Write out newly defined component
	ArgusCodeGeneratorUtil::WriteOutFile(std::string(cStrDefinitionsDirectory).append(inputString.append(".h")), parsedLines);
}
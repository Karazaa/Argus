// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSComponentAdder.h"

#define LOCTEXT_NAMESPACE "FArgusCodeGeneratorModule"

TSharedRef<SDockTab> ArgusECSComponentAdder::OnSpawnPluginTab(const FSpawnTabArgs& spawnTabArgs)
{
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

}

void ArgusECSComponentAdder::OnCheckBoxChecked(ECheckBoxState checkBoxState)
{

}

void ArgusECSComponentAdder::OnClicked()
{

}
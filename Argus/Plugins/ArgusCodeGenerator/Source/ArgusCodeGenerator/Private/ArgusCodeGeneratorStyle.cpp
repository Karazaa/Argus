// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCodeGeneratorStyle.h"
#include "ArgusCodeGenerator.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FArgusCodeGeneratorStyle::StyleInstance = nullptr;

void FArgusCodeGeneratorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FArgusCodeGeneratorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FArgusCodeGeneratorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ArgusCodeGeneratorStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon50x50(50.0f, 50.0f);

TSharedRef< FSlateStyleSet > FArgusCodeGeneratorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ArgusCodeGeneratorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ArgusCodeGenerator")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ArgusCodeGenerator.PluginAction", new IMAGE_BRUSH(TEXT("ArgusCodeGeneratorPluginLogo"), Icon50x50));
	return Style;
}

void FArgusCodeGeneratorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FArgusCodeGeneratorStyle::Get()
{
	return *StyleInstance;
}

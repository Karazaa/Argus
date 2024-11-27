// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "Logging/LogVerbosity.h"

#define ARGUS_LOG(CategoryName, Verbosity, Format, ...) \
do \
{ \
	UE_LOG(CategoryName, Verbosity, Format, ##__VA_ARGS__); \
	ArgusLogging::ShowEditorNotification(FString::Printf(Format, ##__VA_ARGS__), FString{}, ELogVerbosity::Verbosity); \
} \
while (0);

DECLARE_LOG_CATEGORY_EXTERN(ArgusECSLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusInputLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusStaticDataLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusUnrealObjectsLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusTestingLog, Display, All);

static TAutoConsoleVariable<bool> CVarEnableVerboseArgusInputLogging(TEXT("Argus.Input.EnableVerboseLogging"), false, TEXT(""));
static TAutoConsoleVariable<bool> CVarEnableVerboseTestLogging(TEXT("Argus.Input.EnableVerboseTestLogging"), false, TEXT(""));

namespace ArgusLogging
{
	void ShowEditorNotification(const FString& text, const FString& subText = FString{}, ELogVerbosity::Type logVerbosity = ELogVerbosity::Error, float durationSeconds = 5.0f);
	void LogInvalidComponentReferences(const WIDECHAR* functionName, const WIDECHAR* objectName);
}
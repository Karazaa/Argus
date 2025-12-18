// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusMacros.h"
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

#define ARGUS_ERROR_NULL(CategoryName, VariableName) \
ARGUS_LOG(CategoryName, Error, TEXT("[%s] The variable, %s, is null!"), ARGUS_FUNCNAME, ARGUS_NAMEOF(VariableName));

#define ARGUS_RETURN_ON_NULL(VariableName, CategoryName) \
if (UNLIKELY(!VariableName)) \
{ \
	ARGUS_ERROR_NULL(CategoryName, VariableName); \
	return;\
}

#define ARGUS_RETURN_ON_NULL_POINTER(VariableName, CategoryName) \
if (UNLIKELY(!VariableName)) \
{ \
	ARGUS_ERROR_NULL(CategoryName, VariableName); \
	return nullptr;\
}

#define ARGUS_RETURN_ON_NULL_BOOL(VariableName, CategoryName) \
if (UNLIKELY(!VariableName)) \
{ \
	ARGUS_ERROR_NULL(CategoryName, VariableName); \
	return false;\
}

#define ARGUS_RETURN_ON_NULL_VALUE(VariableName, CategoryName, Value) \
if (UNLIKELY(!VariableName)) \
{ \
	ARGUS_ERROR_NULL(CategoryName, VariableName); \
	return Value;\
}

DECLARE_LOG_CATEGORY_EXTERN(ArgusECSLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusInputLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusMemoryLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusStaticDataLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusTestingLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusUILog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusUnrealObjectsLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusUtilitiesLog, Display, All);

static TAutoConsoleVariable<bool> CVarEnableVerboseArgusInputLogging(TEXT("Argus.Input.EnableVerboseLogging"), false, TEXT(""));
static TAutoConsoleVariable<bool> CVarEnableVerboseTestLogging(TEXT("Argus.Input.EnableVerboseTestLogging"), false, TEXT(""));

namespace ArgusLogging
{
	void ShowEditorNotification(const FString& text, const FString& subText = FString{}, ELogVerbosity::Type logVerbosity = ELogVerbosity::Error, float durationSeconds = 5.0f);
	void LogInvalidComponentReferences(const WIDECHAR* functionName, const WIDECHAR* objectName);
}
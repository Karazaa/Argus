// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ArgusECSLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusInputLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusStaticDataLog, Display, All);
DECLARE_LOG_CATEGORY_EXTERN(ArgusUnrealObjectsLog, Display, All);

static TAutoConsoleVariable<bool> CVarEnableVerboseArgusInputLogging(TEXT("Argus.Input.EnableVerboseLogging"), false, TEXT(""));
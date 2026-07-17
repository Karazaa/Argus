// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "HAL/IConsoleManager.h"

class ArgusCVars
{
public:
	static TAutoConsoleVariable<bool> CVarEnableVerboseArgusInputLogging;
	static TAutoConsoleVariable<bool> CVarEnableVerboseTestLogging;

#if !UE_BUILD_SHIPPING
	static TAutoConsoleVariable<bool> CVarDrawECSDebugger;
	static TAutoConsoleVariable<bool> CVarDrawMemoryDebugger;
	static TAutoConsoleVariable<bool> CVarShowObstacleDebug;
	static TAutoConsoleVariable<bool> CVarDrawSaveManagerDebugger;
#endif //!UE_BUILD_SHIPPING
};
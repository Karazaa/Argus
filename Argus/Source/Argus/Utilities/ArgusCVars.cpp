// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusCVars.h"

TAutoConsoleVariable<bool> ArgusCVars::CVarEnableVerboseArgusInputLogging = TAutoConsoleVariable<bool>(TEXT("Argus.Input.EnableVerboseLogging"), false, TEXT(""));
TAutoConsoleVariable<bool> ArgusCVars::CVarEnableVerboseTestLogging = TAutoConsoleVariable<bool>(TEXT("Argus.Test.EnableVerboseTestLogging"), false, TEXT(""));

#if !UE_BUILD_SHIPPING
TAutoConsoleVariable<bool> ArgusCVars::CVarDrawECSDebugger = TAutoConsoleVariable<bool>(TEXT("Argus.Debug.ECS"), false, TEXT("Whether or not the ECS ImGui debugger should be drawn."));
TAutoConsoleVariable<bool> ArgusCVars::CVarDrawMemoryDebugger = TAutoConsoleVariable<bool>(TEXT("Argus.Debug.Memory"), false, TEXT("Whether or not the Memory ImGui debugger should be drawn."));
TAutoConsoleVariable<bool> ArgusCVars::CVarShowObstacleDebug = TAutoConsoleVariable<bool>(TEXT("Argus.SpatialPartitioning.ShowAvoidanceObstacleDebug"), false, TEXT(""));
TAutoConsoleVariable<bool> ArgusCVars::CVarDrawSaveManagerDebugger = TAutoConsoleVariable<bool>(TEXT("Argus.Debug.SaveManager"), false, TEXT("Whether or not the SaveManager ImGui debugger should be drawn."));
#endif //!UE_BUILD_SHIPPING